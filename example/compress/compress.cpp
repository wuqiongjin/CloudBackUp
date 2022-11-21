#include <iostream>
#include <string>
#include <fstream>
#include "bundle.h"
using namespace std;

int main(int argc, char* argv[])
{
  if(argc != 3){
    return -1;
  }
  cout << "压缩文件名称: " << argv[1] << endl;
  cout << "压缩后文件名称: " << argv[2] << endl;
  
  string before_name = argv[1];
  string after_name = argv[2];

  fstream fs(before_name, ios::in | ios::binary);
  fs.seekg(0, fs.end);  //先将文件指针移动到结尾处
  int len = fs.tellg(); //计算文件的大小
  fs.seekg(0, fs.beg);
  string buffer(len, '\0'); //一会pack要使用buffer
  fs.read(&buffer[0], len); //read函数中第二个参数需要传递读取的字符个数, 所以我们之前必须先计算文件的长度(大小)
  fs.close();

  string packed = bundle::pack(bundle::ZPAQ, buffer);
  
  //将压缩完的内存文件 存储到 磁盘当中
  fs.open(after_name, ios::out | ios::binary);
  fs.write(&packed[0], packed.size());
  fs.close();
  return 0;
}
