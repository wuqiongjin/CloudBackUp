#include <iostream>
#include <fstream>
#include <string>
#include "bundle.h"
using namespace std;

int main(int argc, char* argv[])
{
  if(argc != 3){
    std::cerr << "Error input!" << std::endl;
    return -1;
  }
  string before_name = argv[1];
  string after_name = argv[2];
  std::cout << "压缩包文件名称: " << before_name << std::endl;
  std::cout << "解压后的文件名称: " << after_name << std::endl;

  fstream fs;
  fs.open(before_name, ios::in | ios::binary);
  fs.seekg(0, ios::end);
  int len = fs.tellg();
  fs.seekg(0, ios::beg);
  
  string buffer(len, '\0');
  fs.read(&buffer[0], len);
  
  string unpacked = bundle::unpack(buffer);
  fs.close();

  fs.open(after_name, ios::out | ios::binary);
  fs.write(&unpacked[0], unpacked.size());
  fs.close();
  return 0;
}
