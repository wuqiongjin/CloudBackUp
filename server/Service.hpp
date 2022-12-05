#pragma once
#include "DataManager.hpp"
#include "httplib.h"
#include "ThreadPool.hpp"
#include <locale>

extern CloudBackup::DataManager* _datam;

namespace CloudBackup{
  class Service{
    public:
      Service(){
        Config* cf = Config::GetInstance();
        _server_ip = cf->GetServerIP();
        _server_port = cf->GetServerPort();
        _download_prefix = cf->GetDownloadPrefix();
        _access_prefix = cf->GetAccessPrefix();
      }
      
      //文件上传请求
      //1. 通过request获取文件数据(file.filename 与 file.content)
      //2. 解析filename, 为filename拼接backupDir前缀
      //Extra: 字符串编码格式 GBK -> UTF-8(后续我会调整客户端的编码格式)
      //3. 判断是否是xxx_dir.stru目录结构文件; 如果是, 则创建对应目录树结构
      //4. 写入文件
      //5. 修改数据备份信息
      static void Upload(const httplib::Request& req, httplib::Response& rsp){
        //1. 通过request获取文件数据(file.filename 与 file.content)
        auto ret = req.has_file("file");  //判断文件有无上传的文件区域
        if(ret == false){
          rsp.status = 400;
          return;
        }
        const auto& file = req.get_file_value("file");

        //2. 解析filename, 为filename拼接backupDir前缀
        Config* cf = Config::GetInstance();
        std::string realpath = cf->GetBackupDir() + file.filename;  //注意, 这里的backupDir路径是: ./backupDir/  因此客户端上传的文件名不需要携带'/'前缀, 形如:myWork/a.txt
        std::string content = file.content;

        //Extra:将字符串编码由GBK转为UTF-8
        std::string root = cf->GetBackupDir() + file.filename.substr(0, file.filename.find("/"));
        fs::path p_root{root, std::locale("zh_CN.gbk")};
        auto utf_root = p_root.u8string();
        fs::path p_filename{realpath, std::locale("zh_CN.gbk")};
        auto utf_realpath = p_filename.u8string();
        fs::path p_content{content, std::locale("zh_CN.gbk")};
        auto utf_content = p_content.u8string();
        
        //3. 判断是否是xxx_dir.stru目录结构文件; 如果是, 则创建对应目录树结构
        size_t pos = file.filename.rfind("_dir.stru");
        if(pos != std::string::npos){
          //3.1 创建相应目录结构
          auto dir_tree = FileUtil::Split(utf_content, "\n");
          //3.2 为目录结构的每条目录信息, 拼接backupDir前缀
          for(auto& dir : dir_tree)
          {
            std::string tmp = cf->GetBackupDir() + dir;
            std::cerr << tmp << std::endl;
            fs::create_directories(tmp);
          }

          //3.3 递归遍历备份目录, 将备份目录下的所有目录信息添加到哈希表里(持久化存储到cloud.dat中), 以便于后续showlist
          //别忘了添加用户备份的根目录!!!
          BackupInfo root_bi;
          root_bi.NewBackupInfo(utf_root);
          _datam->Insert(root_bi);

          for(auto& p : fs::recursive_directory_iterator(utf_root))
          {
            BackupInfo bi;
            bi.NewBackupInfo(p.path().string());
            //std::cerr << bi._url << std::endl;
            _datam->Insert(bi);
          }
          return;
        }
        std::cerr << utf_realpath << std::endl;
        
        //4. 写入文件
        //FileUtil fu(realpath);
        //fu.SetContent(content);
        FileUtil fu(utf_realpath);
        fu.SetContent(utf_content);
 
        //5. 修改数据备份信息
        BackupInfo bi;
        bi.NewBackupInfo(utf_realpath);
        _datam->Insert(bi);
      }

      //列表展示请求
      //1. 读取数据备份信息
      //2. 根据备份信息, 组织HTML文件数据以用于展示信息
      //3. 填充响应对象rsp
      static void ShowList(const httplib::Request& req, httplib::Response& rsp){
        //1. 读取数据备份信息
        std::vector<BackupInfo> fileInfo;
        //_datam->GetAll(fileInfo);
        Config* cf = Config::GetInstance();
        _datam->GetCurrentAll(cf->GetBackupDir(), fileInfo);
        //std::cerr << fileInfo.size() << std::endl;

        //2. 根据备份信息, 组织HTML文件数据以用于展示信息
        std::stringstream ss;
        ss << "<html><head><meta charset='UTF-8'><title>Download Page</title></head>";
        ss << "<body><h1>Download</h1><table>";
        ss << "<h2><tr><td align='right'>FileName</td><td align='right'>LastModifyTime</td><td align='right'>FileSize</td></tr></h2>";
        for(auto& fi : fileInfo){
          FileUtil fu(fi._real_path);
          ss << "<tr>";
          //2.1 目录文件的显示
          if(fi._file_type == false){
            ss << "<td><a href='" << fi._url << "'>" << fu.FileName() + "/" << "</a></td>";
            ss << "<td align='right'>" << fu.Time2String(fi._mtime) << "</td>";
            ss << "<td align='right'>" << "-" << "</td>";
          }
          else{
            //2.2 普通文件的显示
            ss << "<td><a href='" << fi._url << "'>" << fu.FileName() << "</a></td>";
            ss << "<td align='right'>" << fu.Time2String(fi._mtime) << "</td>";
            //ss << "<td align='right'>" << fu.FileSize() / 1024 << "KB</td>";  //由于文件可能被压缩, 此时real_path路径下可能不存在该文件, 我们在使用fu.FileSize()时涉及访问该文件。因此会出问题!
            ss << "<td align='right'>" << fi._fsize / 1024 << "KB</td>";
          }

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
        //(这边不推荐使用线程池, 原因1:可能出现写文件输入的时候, 文件还没被解压完毕, 所以这里只能阻塞等待解压完毕, 然后再去给客户端填写响应;   原因2: 其次就是如果使用线程池, 则需要在Download里面定义线程池对象, 每次Download都需要创建和销毁一个, 开销很大)
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

      
      //访问目录文件的响应
      //1. 根据URL到哈希表中找到对应的数据文件信息
      //2. 获取当前目录的"当前层"的所有文件信息
      //3. 根据备份信息, 组织HTML文件数据以用于展示信息
      //4. 填充响应对象rsp
      static void Access(const httplib::Request& req, httplib::Response& rsp){
        BackupInfo bi;
        auto ret = _datam->GetOneByURL(req.path, bi);
        if(ret == false){
          rsp.status = 404;
          rsp.set_header("content-type", "text/html");
          rsp.body = "Resource Not Found!";
        }

        //std::vector<std::string> contained_files;
        //FileUtil fu(bi._real_path);
        //fu.ScanCurrentDirectory(contained_files);
        std::vector<BackupInfo> contained_files;
        _datam->GetCurrentAll(bi._real_path, contained_files);

        //3.
        std::string backupDir = Config::GetInstance()->GetBackupDir();
        size_t pos = bi._real_path.find(backupDir);
        std::string subdir = bi._real_path.substr(pos + backupDir.size());
        
        std::stringstream ss;
        ss << "<html><head><meta charset='UTF-8'><title>Download Page</title></head>";
        ss << "<body><h1>Download/" << subdir << "</h1><table>";
        for(auto& fi : contained_files)
        {
          FileUtil fu(fi._real_path);
          ss << "<tr>";
          //3.1 目录文件的显示
          if(fi._file_type == false){
            ss << "<td><a href='" << fi._url << "'>" << fu.FileName() + "/" << "</a></td>";
            ss << "<td align='right'>" << fu.Time2String(fi._mtime) << "</td>";
            ss << "<td align='right'>" << "-" << "</td>";
          }
          else{
            //2.2 普通文件的显示
            ss << "<td><a href='" << fi._url << "'>" << fu.FileName() << "</a></td>";
            ss << "<td align='right'>" << fu.Time2String(fi._mtime) << "</td>";
            ss << "<td align='right'>" << fi._fsize / 1024 << "KB</td>";
          }

          ss << "</tr>";
        }
        ss << "</table></body></html>";

        //4.
        rsp.status = 200;
        rsp.set_header("content-type", "text/html");
        rsp.body = ss.str();
      }


      bool RunModule(){
        _svr.Post("/upload", Upload);
        _svr.Get("/", ShowList);
        _svr.Get("/showlist", ShowList);
        std::string download_url = _download_prefix + "(.*)"; //后面的是正则匹配任意个字符
        _svr.Get(download_url, Download);
        std::string access_url = _access_prefix + "(.*)";
        _svr.Get(access_url, Access);

        _svr.listen(_server_ip, _server_port);
        return true;
      }

    private:
      std::string _server_ip;
      int _server_port;
      std::string _download_prefix;
      std::string _access_prefix;
      httplib::Server _svr;
  };
}
