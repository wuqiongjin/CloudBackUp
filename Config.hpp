#pragma once
#include "Util.hpp"
#include <mutex>
constexpr auto CONFIG_FILE = "./Cloud.conf";

namespace CloudBackup{
  class Config{
    public:
      Config* GetInstance(){
        if(nullptr == _inst){
          _mtx.lock();
          if(nullptr == _inst){
            _inst = new Config();
          }
          _mtx.unlock();
        }
        return _inst;
      }

      int GetHotTime()  { return _hot_time;  }
      std::string GetServerIP() { return _server_ip;  }
      int GetServerPort() { return _server_port;  }
      std::string GetPackSuffix() { return _pack_suffix;  }
      std::string GetPackDir()  { return _pack_dir;  }
      std::string GetBackupDir() { return _backup_dir;  }
      std::string GetBackupInfoList() { return _backup_info_list;  }
      std::string GetDownloadPrefix() { return _download_prefix;  }
    private:
      //从CONFIG_FILE中读取配置文件信息
      bool ReadConfigFile(){
        CloudBackup::FileUtil fu(CONFIG_FILE);
        std::string content;
        fu.GetContent(content);
        //反序列化
        //我们手里现在有Json格式字符串, 需要将其反序列化为Json对象, 然后从Json对象中获取成员的值
        Json::Value root;
        if(CloudBackup::JsonUtil::Deserialize(content, root) == false){
          std::cerr << "ReadConfigFile: Json Deserialize failed!" << std::endl;
          return false;
        }
        _hot_time = root["hot_time"].asInt();
        _server_ip = root["server_ip"].asString();
        _server_port = root["server_port"].asInt();
        _pack_suffix = root["pack_suffix"].asString();
        _pack_dir = root["pack_dir"].asString();
        _backup_dir = root["backup_dir"].asString();
        _backup_info_list = root["backup_info_list"].asString();
        _download_prefix = root["download_prefix"].asString();
        return true;
      }
    private:
      Config(){
        ReadConfigFile();
      }
      static Config* _inst;
      static std::mutex _mtx;
    private:
      int _hot_time;  //热点时间, 超过这个时间没有访问, 进行压缩备份
      std::string _server_ip; //服务器IP
      int _server_port;       //服务器端口号
      std::string _pack_suffix; //压缩包后缀名
      std::string _pack_dir;    //压缩包的存放路径
      std::string _backup_dir;  //备份文件的存放路径
      std::string _backup_info_list;  //备份文件信息列表(这里把备份文件的信息存放到了cloud.dat文件里了)
      std::string _download_prefix;   //下载备份文件时需要为下载请求URL添加的前缀
  };
  
  Config* Config::_inst = nullptr;
  std::mutex Config::_mtx;
};
