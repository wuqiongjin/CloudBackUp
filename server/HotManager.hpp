#pragma once
#include <ctime>
#include <unistd.h>
#include "DataManager.hpp"
#include "ThreadPool.hpp"

extern CloudBackup::DataManager* _datam;

namespace CloudBackup{
  class HotManager{
    public:
      HotManager()
        :_tp(new ThreadPool())
      {
        //读取配置文件信息到 成员变量中
        Config* cf = Config::GetInstance();
        _hot_time = cf->GetHotTime();
        _back_dir = cf->GetBackupDir();
        _pack_dir = cf->GetPackDir();
        _pack_suffix = cf->GetPackSuffix();

        FileUtil back_fu(_back_dir);
        FileUtil pack_fu(_pack_dir);
        back_fu.CreateDirectory();  //CreateDirectory里面会自动判断目录是否已经存在
        pack_fu.CreateDirectory();
      }
      
      //热点管理模块的核心功能 --- 判断非热点文件然后进行压缩
      //1. 获取备份目录下的所有文件(遍历备份目录)
      //2. 逐个判断文件是否是非热点文件 + 文件是否没被压缩 
                                        //(在我们引入了线程池后, 由于我们直接将压缩任务交给了线程池中的线程, 它们会异步的帮我们执行任务, 因此主线程会直接循环到下一次, 所以下一次遍历目录的时候, 很可能原文件还没有被压缩完毕, 即backupDir下面还有该文件, 所以我们这里必须还要额外加上pack_flag标志位的判断, 只有它为false才去压缩)
      
      //3. 通过数据信息管理模块获取到非热点文件各自对应的压缩路径_pack_dir, 如果该文件不存在, 则更新备份信息  
                    //(这个情况就是实际的目录中存在该文件, 但是数据信息模块中保存备份信息的持久化文件中没有存储该文件的信息, 这里我们应该更新备份信息到这个持久化存储文件中[就是到备份文件数据信息列表中]), 如何制造出这种情况呢??? 我们服务端，直接将文件移动到backupDir, 此时我们不是通过客户端的上传进行的, 因此客户端上传文件时所记录在内存中哈希表里的数据是不会被更新的, 因此我们需要手动更新哈希表的数据。
      //4. 对非热点文件进行压缩, 并将压缩包放入packdir
      //5. 删除原文件
      //6. 修改备份信息
      bool RunModule(){
        while(1){
          //1.
          std::vector<std::string> files;
          FileUtil fu(_back_dir);
          fu.ScanDirectory(files);  //递归backupDir获取文件信息

          for(auto file : files){

            BackupInfo bi;
            bool ret = _datam->GetOneByRealPath(file, bi);  //这里将第3部分需要做的事情提前取出来了(因为我们需要获取到文件中的压缩标志位)
            //2. 判断 是否为普通文件 + 是否为非热点文件 + 文件是否没有被压缩
            if(fs::is_directory(file) == false && HotJudge(file) == false && bi._pack_flag == false){
              //3. 通过数据信息管理模块获取信息
              //如果备份信息模块中不存在该文件(而实际目录中存在该文件), 更新备份信息文件
              if(ret == false){
                bi.NewBackupInfo(file);
                _datam->Insert(bi);
              }
              //4. 压缩文件, 并将压缩包放入packdir
              FileUtil fu(file);
              auto f_compress = std::bind(&FileUtil::Compress, fu, bi._pack_dir);
              auto f_remove = std::bind(&FileUtil::RemoveFile, fu);
              _tp->PushTask(f_compress);  //将任务推送到线程池
              _tp->PushTask(f_remove);
              //fu.Compress(bi._pack_dir);  //Compress参数是压缩包的路径。而压缩包的路径我们在数据信息模块中保存了。这里压缩完毕后,会直接在pack_dir里面生成压缩包。
              //5. 删除原文件
              //fu.RemoveFile();
              //6. 修改备份信息
              bi._pack_flag = true; //设置压缩标志位为true, 表示该文件已经被压缩了
              _datam->Update(bi);
            }
          }
          usleep(1000); //避免空目录的循环遍历, 导致CPU资源消耗过高
        }
      }

    private:
      //判断是否是热点文件。热点文件:true;  非热点文件:false
      bool HotJudge(const std::string& file){
        FileUtil fu(file);
        time_t cur_time = time(NULL);
        time_t last_atime = fu.LastAccessTime();
        if(cur_time - last_atime > _hot_time){
          return false;
        }
        return true;
      }

    private:
      int _hot_time;  //热点判断时间差 30s
      std::string _back_dir;
      std::string _pack_dir;
      std::string _pack_suffix;
      std::shared_ptr<ThreadPool> _tp;
  };
}
