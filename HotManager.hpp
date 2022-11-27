#pragma once
#include <ctime>
#include <unistd.h>
#include "DataManager.hpp"

extern CloudBackup::DataManager* _datam;

namespace CloudBackup{
  class HotManager{
    public:
      HotManager(){
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
      //2. 逐个判断文件是否是非热点文件
      //3. 通过数据信息管理模块获取到非热点文件各自对应的压缩路径_pack_dir, 如果该文件不存在, 则更新备份信息  (这个情况就是实际的目录中存在该文件, 但是数据信息模块中保存备份信息的持久化文件中没有存储该文件的信息, 这里我们应该更新备份信息到这个持久化存储文件中[就是到备份文件数据信息列表中])
      //4. 对非热点文件进行压缩, 并将压缩包放入packdir
      //5. 删除原文件
      //6. 修改备份信息
      bool RunModule(){
        while(1){
          //1.
          std::vector<std::string> files;
          FileUtil fu(_back_dir);
          fu.ScanDirectory(files);

          //2.
          for(auto file : files){

            if(HotJudge(file) == false){
              //3. 通过数据信息管理模块获取信息
              BackupInfo bi;
              bool ret = _datam->GetOneByRealPath(file, bi);
              //如果备份信息模块中不存在该文件(而实际目录中存在该文件), 更新备份信息文件
              if(ret == false){
                bi.NewBackupInfo(file);
                _datam->Insert(bi);
              }
              //4. 压缩文件, 并将压缩包放入packdir
              FileUtil fu(file);
              fu.Compress(bi._pack_dir);  //Compress参数是压缩包的路径。而压缩包的路径我们在数据信息模块中保存了。这里压缩完毕后,会直接在pack_dir里面生成压缩包。
              //5. 删除原文件
              fu.RemoveFile();
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
  };
}
