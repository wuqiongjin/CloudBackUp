#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <ctime>
#include <experimental/filesystem>
#include <jsoncpp/json/json.h>
#include <sys/stat.h>
#include "./liba/include/bundle.h"
namespace fs = std::experimental::filesystem;

namespace CloudBackup{
  //文件实用工具类
  class FileUtil{
    public:
      FileUtil(const std::string& name) 
        :_filename(name) 
      {}

      bool RemoveFile(){
        return fs::remove(_filename);
      }

      int64_t FileSize(){
        if(stat(_filename.c_str(), &_st) < 0){
          std::cerr << "Get FileSize Failed!" << std::endl;
          return -1;
        }
        return _st.st_size;
      }
      
      time_t LastModifyTime(){
        if(stat(_filename.c_str(), &_st) < 0){
          std::cerr << "Get LastModifyTime Failed!" << std::endl;
          return -1;
        }
        return _st.st_mtime;
      }

      time_t LastAccessTime(){
        if(stat(_filename.c_str(), &_st) < 0){
          std::cerr << "Get LastAccessTime Failed!" << std::endl;
          return -1;
        }
        return _st.st_atime;
      }

      std::string Time2String(time_t t){
        return ctime(&t);  //该函数返回一个char*
      }

      //获取文件名称(去除路径前缀)
      //该函数的作用是: 将成员变量'_filename'裁剪为文件名(去除前面的路径), 它不涉及真的去访问该路径下的文件!!!(因此即便文件不存在, 如: 文件已经被压缩了。该函数也能裁剪出文件名)
      //因此, 我们可以把它当做一个工具函数
      std::string FileName(){
        // ../abc/test.txt  获取到最后的test.txt
        auto pos = _filename.rfind('/');
        if(pos == std::string::npos){
          return _filename;
        }
        return _filename.substr(pos + 1);
      }

      //读取当前类保存的filename的内容, 但是指定了从某个位置处开始、确定长度 的内容
      //第一个参数content是一个输出型参数, 外界用于获取文件内容
      bool GetPosLenContent(std::string& output_content, size_t pos, size_t len){
        size_t fz = this->FileSize();
        if(pos + len > fz){
          std::cerr << "Read Size exceed FileSize!" << std::endl;
          return false;
        }

        output_content.resize(len - pos, '\0');  //!!! 这里必须要先resize, 不然后面read的时候没法修改string
        
        std::ifstream ifs(_filename, std::ios::binary);  //ifs默认读文件 (fstream需要指定读写)
        if(ifs.is_open() == false){
          std::cerr << "open ReadFile failed!" << std::endl;
          return false;
        }
        ifs.seekg(pos, std::ios::beg);  //从beg处往后偏移pos长度
        ifs.read(&output_content[0], len);
        ifs.close();
        return true;
      }

      //读取当前类保存的filename的全部内容
      bool GetContent(std::string& output_content){
        return this->GetPosLenContent(output_content, 0, this->FileSize());
      }

      //写入到当前类所保存的filename文件中, 写入的内容是content
      bool SetContent(const std::string& content){
        std::ofstream ofs(_filename, std::ios::binary);
        if(ofs.is_open() == false){
          std::cerr << "open WriteFile failed!" << std::endl;
          return false;
        }
        ofs.write(&content[0], content.size());
        ofs.close();
        return true;
      }

      //压缩文件: 压缩当前类保存的文件 (参数packname表示要生成的压缩包的名称)
      //1. 读取文件内容 --- GetContent()
      //2. 压缩文件
      //3. 将压缩的文件写入磁盘中 --- 先创建一个新的FileUtil类, 然后进行SetContent()
      bool Compress(const std::string& packname){
        std::string content;
        if(this->GetContent(content) == false){
          std::cerr << "Compress: GetContent failed!" << std::endl;
          return false;
        }

        std::string packed = bundle::pack(bundle::ZPAQ, content);
        
        FileUtil fu(packname);
        if(fu.SetContent(packed) == false){
          std::cerr << "Compress: SetContent failed!" << std::endl;
          return false;
        }
        return true;
      }

      //解压文件: 解压当前类保存的压缩包文件 (filename为解压后要生成的文件名称)
      bool UnCompress(const std::string& filename){
        std::string content;
        if(this->GetContent(content) == false){
          std::cerr << "UnCompress: GetContent failed!" << std::endl;
          return false;
        }

        std::string unpacked = bundle::unpack(content);

        FileUtil fu(filename);
        if(fu.SetContent(unpacked) == false){
          std::cerr << "UnCompress: SetContent failed!" << std::endl;
          return false;
        }
        return true;
      }

      //!下面三个函数是针对"目录"文件相关的操作函数!
      //判断filename这个目录是否存在
      bool Exists(){
        return fs::exists(_filename);
      }

      //当filename目录不存在时, 创建该目录
      bool CreateDirectory(){
        //if(this->Exists()){
        //  return true;
        //}
        return fs::create_directory(_filename); //它会自动判断, 如果目录已经存在, 不会做任何处理; 目录不存在则创建目录
      }

      //遍历filename目录下的所有文件, 将文件存储到array数组中(输出型参数)
      //注意: 存入到array的文件名称是'相对路径', 我们不是只存文件的名字, 我们还需要存它的前缀路径(相对的)
      //获取当前目录及其子目录的所有文件信息
      //被使用的地方: HotManager.hpp 压缩模块
      bool ScanDirectory(std::vector<std::string>& output_array){
        //扩展: 这里使用recursive_directory_iterator可以递归遍历到filename目录下的所有文件(包含其子目录的文件), 那么是否可以把目录也上传到服务器呢?
        for(auto& p : fs::recursive_directory_iterator(_filename)){
          //判断是否是目录文件, 将非目录文件添加到array数组中
          if(fs::is_directory(p) == false){
            output_array.emplace_back(fs::path(p).relative_path().string());
          }
        }
        return true;
      }

      ////只获取当前层(不会递归到子目录)
      ////被使用的地方: Service.hpp Access目录的地方
      //bool ScanCurrentDirectory(std::vector<std::string>& output_array){
      //  for(auto& p : fs::directory_iterator(_filename)){
      //    output_array.emplace_back(fs::path(p).relative_path().string());
      //  }
      //  return true;
      //}

public:
      static std::vector<std::string> Split(const std::string& s, const std::string& sep){
        std::vector<std::string> res;
        size_t pos = 0;
        size_t prev = 0;
        while((pos = s.find(sep, prev)) != std::string::npos)
        {
          if(prev == pos){
            prev = pos + sep.size();
            continue;
          }
          res.emplace_back(s.substr(prev, pos - prev));
          prev = pos + sep.size();
        }

        //有一说一, 我们可以控制客户端, 让客户端的.stru文件最后一行多一个'\n', 然后就不需要这个判断了
        if(prev < s.size()){
          res.emplace_back(s.substr(prev));
        }

        return res;
      }

    private:
      std::string _filename;
      struct stat _st;
  };


  class JsonUtil{
    public:
    //序列化: 将Json对象转化为Json格式字符串 (第二个参数为输出型参数)
    static bool Serialize(Json::Value& obj, std::string& output_JsonString){
      Json::StreamWriterBuilder swbuilder;
      std::unique_ptr<Json::StreamWriter> sw(swbuilder.newStreamWriter());
      std::ostringstream oss;
      //将转化的Json字符串写入了ostream流中
      if(sw->write(obj, &oss) != 0){
        std::cerr << "Json Serialize write failed!" << std::endl;
        return false;
      } 
      output_JsonString = oss.str();
      return true;
    }

    //反序列化: 将Json格式字符串转化为Json对象 (第二个参数为输出型参数)
    static bool Deserialize(std::string& JsonString, Json::Value& output_obj){
      Json::CharReaderBuilder crbuilder;
      std::unique_ptr<Json::CharReader> cr(crbuilder.newCharReader());
      std::string err;
      bool ret = cr->parse(JsonString.c_str(), JsonString.c_str() + JsonString.size(), &output_obj, &err);
      if(ret == false){
        std::cerr << "Json Deserialize parse failed!" << std::endl;
        return false;
      }
      return true;
    }
  };
}
