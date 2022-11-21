#include "httplib.h"

#define SERVER_IP "152.136.211.148"
#define SERVER_PORT 2333

int main()
{
  using namespace httplib;
  Client cli(SERVER_IP, SERVER_PORT);  
  
  MultipartFormData item;
  item.name = "file";
  item.filename = "hello.txt";
  item.content = "Hello World!, This is the content!";  //上传文件时，这里就是具体的文件内容
  item.content_type = "text/plain";

  MultipartFormDataItems item_array;
  item_array.emplace_back(item);

  auto res = cli.Post("/upload", item_array);
  std::cout << res->status << std::endl;
  std::cout << res->body << std::endl;

  return 0;
}
