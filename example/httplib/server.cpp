#include "httplib.h"
#include <string>

int main()
{
  using namespace httplib;
  httplib::Server svr;
  svr.Get("/hello", [](const Request&, Response& rsp){
      rsp.status = 200;
      rsp.set_content("Hello World!", "text/plain");
  });

  svr.Get(R"(/numbers/(\d+))", [](const Request& req, Response& rsp){
      auto num = req.matches[1];
      rsp.status = 200;
      rsp.set_content(num, "text/plain");
  });

  svr.Post("/upload", [](const Request& req, Response& rsp){
      auto size = req.files.size();
      auto ret = req.has_file("file");
      if(ret == false){
        std::cout << "Not File Upload!\n";
        rsp.status = 400;
        return;
      }
      const auto& file = req.get_file_value("file");
      rsp.body.clear();
      rsp.body = file.filename; //文件名称
      rsp.body += "\n";
      rsp.body += file.content; //文件内容
      rsp.set_header("content-type", "text/plain");
      rsp.status = 200;
      
      std::cout << rsp.body; 
      std::flush(std::cout);  //这里必须刷新一下缓冲区, 不然body的数据有一部分打印不出来
      
      //std::cout << file.filename << std::endl;
      //std::cout << file.content_type << std::endl;
      //std::cout << file.content << std::endl;
  });
  
  svr.listen("0.0.0.0", 2333);
  return 0;
}
