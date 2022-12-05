#pragma once
#include <unordered_map>
#include <pthread.h>
#include "Util.hpp"     //Config.hpp里面也包含了Util, 其实可以再不用包含Util.hpp了
#include "Config.hpp" 

namespace CloudBackup{
  struct BackupInfo{
    public:
      //其实可以考虑把NewBackupInfo的内容改为BackupInfo的构造函数中, 在创建对象的时候直接传入realpath即可
      bool NewBackupInfo(const std::string& realpath){
        CloudBackup::FileUtil fu(realpath); //这里其实不用就加CloudBackup, 因为在同一个命名空间里
        if(fu.Exists() == false){
          std::cerr << "NewBackupInfo: realpath not exists!" << std::endl;
          return false;
        }

        if(fs::is_directory(realpath)){
          _file_type = false;
        }
        else{
          _file_type = true;
        }

        _pack_flag = false; //新创建的备份文件默认是热点文件, 因此设置压缩标志位为false(没有被压缩)
        _fsize = fu.FileSize();
        _mtime = fu.LastModifyTime();
        _atime = fu.LastAccessTime();
        _real_path = realpath;

        //pack_dir: 备份文件路径:./backdir/a.txt   ->    ./packdir/a.txt.zp
        std::string filename = fu.FileName();
        CloudBackup::Config* cf = CloudBackup::Config::GetInstance();
        std::string packdir = cf->GetPackDir();
        std::string packsuffix = cf->GetPackSuffix();
        _pack_dir = packdir + filename + packsuffix;

        std::string true_path = _real_path.substr(2);
        //普通文件需要添加下载路径前缀/download/
        if(_file_type){
          //URL: 备份文件路径:./backdir/a.txt   ->    /download/a.txt
          std::string download_prefix = cf->GetDownloadPrefix();
          _url = download_prefix + true_path;
        }
        else{
          std::string access_prefix = cf->GetAccessPrefix();
          _url = access_prefix + true_path;
        }
        return true;
      }
    public:
      bool _file_type = true;  //文件类型: true为普通文件; false为目录文件
      bool _pack_flag = false;  //判断文件是否已经被压缩了
      size_t _fsize;    //文件大小
      time_t _mtime;    //文件最后一次修改时间
      time_t _atime;    //文件最后一次访问时间
      std::string _real_path;  //文件实际存放路径 (./backupDir/filename.xxx)
      std::string _pack_dir;  //压缩包存放路径 (./packDir/filename.xxx.zp)
      std::string _url;        //下载文件时的URL请求资源路径
  };

  class DataManager{
    public:
      DataManager(){
        _persistence_file = Config::GetInstance()->GetBackupInfoList();//持久化存储文件(备份文件的数据信息列表)
        pthread_rwlock_init(&_rwlock, nullptr);
        InitLoad();
      }
      ~DataManager(){
        pthread_rwlock_destroy(&_rwlock);
      }
      
      bool Insert(const BackupInfo& info){
        pthread_rwlock_wrlock(&_rwlock);
        _path2InfoTable[info._url] = info;
        pthread_rwlock_unlock(&_rwlock);
        Storage();  //添加、更新新的数据信息后, 进行Storage持久化存储
        return true;
      }

      bool Update(const BackupInfo& info){
        pthread_rwlock_wrlock(&_rwlock);
        _path2InfoTable[info._url] = info;
        pthread_rwlock_unlock(&_rwlock);
        Storage();
        return true;
      }

      //注意: GetOneByURL、GetOneByRealPath、GetAll 这三个函数都是对外提供的'查询'接口。
      //查询意味着: 我们要先执行哈希表的查询操作, 在确保哈希表中存在该数据信息时，才通过对输出型参数进行赋值返回  (这个就是查询的过程)
      //(我为什么会想到上面这个问题, 详情见"GetOneByRealPath -> output_info.NewBackupInfo(realpath)")
      bool GetOneByURL(const std::string& url, BackupInfo& output_info){
        pthread_rwlock_rdlock(&_rwlock);
        auto it = _path2InfoTable.find(url);
        if(it == _path2InfoTable.end()){
          std::cout << "URL not exists!" << std::endl;
          pthread_rwlock_unlock(&_rwlock);
          return false;
        }
        
        output_info = it->second;
        pthread_rwlock_unlock(&_rwlock);
        return true;
      }

      bool GetOneByRealPath(const std::string& realpath, BackupInfo& output_info){
        pthread_rwlock_rdlock(&_rwlock);
        //output_info.NewBackupInfo(realpath);  //其实可以直接调用NewBackupInfo接口, 来直接对output_info的值进行修改。(只不过这样做就失去了意义了, 这个函数是通过传入的realpath，然后到哈希表里面去寻找其对应的BackupInfo数据信息文件)
        for(auto it = _path2InfoTable.begin(); it != _path2InfoTable.end(); ++it)
        {
          if(it->second._real_path == realpath){
            output_info = it->second;
            pthread_rwlock_unlock(&_rwlock);
            return true;
          }
        }
        pthread_rwlock_unlock(&_rwlock);
        return false;
      }

      bool GetCurrentAll(const std::string& cur_path, std::vector<BackupInfo>& output_array){ 
        pthread_rwlock_rdlock(&_rwlock);
        for(auto& p : fs::directory_iterator(cur_path))
        {
          //std::cerr << "fs: " << p.path() << std::endl;
          BackupInfo bi;
          if(GetOneByRealPath(p.path().string(), bi) == false){
            return false;
          }
          auto ret = _path2InfoTable.find(bi._url);
          if(ret != _path2InfoTable.end()){
            output_array.emplace_back(ret->second);
          }
        }
        pthread_rwlock_unlock(&_rwlock);
        return true;
      }

      bool GetAll(std::vector<BackupInfo>& output_array){
        pthread_rwlock_rdlock(&_rwlock);
        for(auto it = _path2InfoTable.begin(); it != _path2InfoTable.end(); ++it)
        {
          output_array.emplace_back(it->second);
        }
        pthread_rwlock_unlock(&_rwlock);
        return true;
      }

      //持久化存储文件函数(在调用完添加数据信息、更新数据信息接口后需要调用持久化存储函数)
      //1. 获取所有数据信息
      //2. 将所有数据信息添加到Json对象当中
      //3. Json序列化
      //4. 写入磁盘文件当中
      bool Storage(){
        //1.
        std::vector<BackupInfo> data;
        GetAll(data);
        //2.
        Json::Value obj;
        for(size_t i = 0; i < data.size(); ++i)
        {
          Json::Value tmp;
          tmp["file_type"] = data[i]._file_type;
          tmp["pack_flag"] = data[i]._pack_flag;
          tmp["fsize"] = static_cast<int>(data[i]._fsize);
          tmp["mtime"] = static_cast<int>(data[i]._mtime);
          tmp["atime"] = static_cast<int>(data[i]._atime);
          tmp["real_path"] = data[i]._real_path;
          tmp["pack_dir"] = data[i]._pack_dir;
          tmp["url"] = data[i]._url;
          obj.append(tmp);
        }
        //3. Json序列化
        std::string jstr;
        JsonUtil::Serialize(obj, jstr);
        //4.
        FileUtil fu(_persistence_file);
        fu.SetContent(jstr);
        return true;
      }

      //初始化加载函数(在启动数据管理模块时会先将持久化的数据文件读取到内存中的哈希表里)
      //1. (在这之前, 要判断cloud.dat文件是否存在, 如果不存在, 直接return即可, 因为没有任何数据信息需要加载)读取持久化数据文件到string里
      //2. 将string进行Json反序列化
      //3. 将数据信息插入到哈希表中
      bool InitLoad(){
        std::string jstr;
        //1.
        FileUtil fu(_persistence_file);
        //持久化存储数据信息文件不存在, 不需要进行初始化加载, 因此直接return
        if(fu.Exists() == false){
          return true;
        }

        fu.GetContent(jstr);
        //2.
        Json::Value obj;
        JsonUtil::Deserialize(jstr, obj);
        //3.
        for(Json::ArrayIndex i = 0; i < obj.size(); ++i)
        {
          BackupInfo bi;
          bi._file_type = obj[i]["file_type"].asBool();
          bi._pack_flag = obj[i]["pack_flag"].asBool();
          bi._fsize = obj[i]["fsize"].asInt();
          bi._mtime = obj[i]["mtime"].asInt();
          bi._atime = obj[i]["atime"].asInt();
          bi._real_path = obj[i]["real_path"].asString();
          bi._pack_dir = obj[i]["pack_dir"].asString();
          bi._url = obj[i]["url"].asString();
          Insert(bi);
        }
        return true;
      }

    private:
      std::string _persistence_file;  //用于进行持久化存储文件的文件名称 --- 对应的就是Config.hpp中的_backup_info_list(备份文件数据信息列表)
      std::unordered_map<std::string, BackupInfo> _path2InfoTable;  //URL路径 to 数据信息 的映射
      pthread_rwlock_t _rwlock; //读写锁
  };
}
