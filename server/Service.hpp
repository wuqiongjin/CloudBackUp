#pragma once
#include "DataManager.hpp"
#include "httplib.h"

extern CloudBackup::DataManager* _datam;

namespace CloudBackup{
  class Service{
    public:
      Service(){
        Config* cf = Config::GetInstance();
        _server_ip = cf->GetServerIP();
        _server_port = cf->GetServerPort();
        _download_prefix = cf->GetDownloadPrefix();
      }
      
      //文件上传请求
      //1. 通过request获取文件数据(file.filename 与 file.content)
      //2. 写入文件
      //3. 修改数据备份信息
      static void Upload(const httplib::Request& req, httplib::Response& rsp){
        //1. 通过request获取文件数据(file.filename 与 file.content)
        auto ret = req.has_file("file");  //判断文件有无上传的文件区域
        if(ret == false){
          rsp.status = 400;
          return;
        }
        const auto& file = req.get_file_value("file");

        //2. 写入文件
        Config* cf = Config::GetInstance();
        std::string realpath = cf->GetBackupDir() + file.filename;
        std::string content = file.content;
        
        FileUtil fu(realpath);
        fu.SetContent(content);
 
        //3. 修改数据备份信息
        BackupInfo bi;
        bi.NewBackupInfo(realpath);
        _datam->Insert(bi);
      }

      //列表展示请求
      //1. 读取数据备份信息
      //2. 根据备份信息, 组织HTML文件数据以用于展示信息
      //3. 填充响应对象rsp
      static void ShowList(const httplib::Request& req, httplib::Response& rsp){
        //1. 读取数据备份信息
        std::vector<BackupInfo> fileInfo;
        _datam->GetAll(fileInfo);

        //2. 根据备份信息, 组织HTML文件数据以用于展示信息
        std::stringstream ss;
        ss << "<html><head><meta charset='UTF-8'><title>Download Page</title></head>";
        ss << "<body><h1>Download</h1><table>";
        for(auto& fi : fileInfo){
          FileUtil fu(fi._real_path);
          ss << "<tr>";
          ss << "<td><a href='" << fi._url << "'>" << fu.FileName() << "</a></td>";
          ss << "<td align='right'>" << fu.Time2String(fi._mtime) << "</td>";
          //ss << "<td align='right'>" << fu.FileSize() / 1024 << "KB</td>";  //由于文件可能被压缩, 此时real_path路径下可能不存在该文件, 我们在使用fu.FileSize()时涉及访问该文件。因此会出问题!
          ss << "<td align='right'>" << fi._fsize / 1024 << "KB</td>";
          ss << "</tr>";
        }
        ss << "</table></body></html>";

        //3. 填充响应对象rsp
        rsp.status = 200;
        rsp.set_header("content-type", "text/html");
        rsp.body = ss.str();
      }

      //ETag是服务器内部用于标识资源唯一性的标志位 --- (生成ETag的规则是服务器自己定义的)
      //这里我们就定义ETag的组成为: filenam-fsize-LastModifyTime
      static std::string GetETag(const BackupInfo& bi){
        FileUtil fu(bi._real_path);
        std::string ETag = fu.FileName();
        ETag += "-";
        ETag += std::to_string(fu.FileSize());
        ETag += "-";
        ETag += std::to_string(fu.LastModifyTime());
        return ETag;
      }

      //文件下载请求
      //1. 获取客户端请求资源的URL路径  (req.path中保存了请求的URL资源路径)
      //2. 根据URL路径，获取文件的备份信息
      //3. 判断该文件是否被压缩
      //  3.1 如果文件被压缩, 进行解压缩, 并删除压缩包
      //  3.2  更新文件备份信息 (仅pack_flag)
      //4. 判断客户端请求中是否包含"If-Range"字段。
      //  4.1 如果包含"If-Range", 则获取客户端请求中的ETag, 并将其与服务端对应文件的ETag进行对比; 相等则设置断点续传标志位为true
      //5. 读去文件内容, 将文件内容赋值给rsp.body响应正文
      //6. 设置响应头部相关字段(ETag : xxx, Accept-Ranges : bytes, Content-Type : application/octet-stream)
      static void Download(const httplib::Request& req, httplib::Response& rsp){
        //1. & 2. : 获取客户端请求资源URL路径; 根据URL路径获取文件备份信息
        BackupInfo bi;
        //bi.NewBackupInfo(req.path);
        auto ret = _datam->GetOneByURL(req.path, bi);  //这里我们是需要去 DataManager 中的哈希表里进行搜索的! 而不是使用NewBackupinfo
        if(ret == false){
          std::cerr << "Service::Download: GetOneByURL failed!" << std::endl;
          return;
        }
        
        //3. 判断文件是否被压缩
        if(bi._pack_flag == true){
          //3.1 文件被压缩, 进行解压缩, 并删除压缩包
          FileUtil fu(bi._pack_dir);  //根据压缩包路径传给FileUtil
          fu.UnCompress(bi._real_path); //real_path是未压缩的文件存放的路径
          fu.RemoveFile();
          bi._pack_flag = false;
          //3.2 更新文件备份信息
          _datam->Update(bi);
        }
        
        //4.
        bool breakpoint_transmit = false;
        std::string client_etag;
        if(req.has_header("If-Range") == true){
          client_etag = req.get_header_value("If-Range");
          if(client_etag == GetETag(bi)){
            breakpoint_transmit = true;
          }
        }

        //5. 读取文件内容, 填充rsp.body
        //在httplib库中, 已经对断点续传进行处理了。所以我们这边可以直接传整个文件
        FileUtil fu(bi._real_path);
        std::string content;
        fu.GetContent(content);
        rsp.body = content;

        //6. 设置响应报头相关字段
        if(breakpoint_transmit == false){
          rsp.status = 200;
          rsp.set_header("content-type", "application/octet-stream");
          rsp.set_header("ETag", GetETag(bi));
          rsp.set_header("Accept-Ranges", "bytes");
        }
        else{
          rsp.status = 206; //传输部分数据(其实也不用我们自己设置, httplib已经为我们做好了)
          rsp.set_header("content-type", "application/octet-stream");
          rsp.set_header("ETag", GetETag(bi));
          rsp.set_header("Accept-Ranges", "bytes");
        }

      }

      bool RunModule(){
        _svr.Post("/upload", Upload);
        _svr.Get("/", ShowList);
        _svr.Get("/showlist", ShowList);
        std::string download_url = _download_prefix + "(.*)"; //后面的是正则匹配任意个字符
        _svr.Get(download_url, Download);

        _svr.listen(_server_ip, _server_port);
        return true;
      }

    private:
      std::string _server_ip;
      int _server_port;
      std::string _download_prefix;
      httplib::Server _svr;
  };
}
