#include "Util.hpp"

void test_Util(std::string& filename)
{
  //测试获取文件相关属性
  //CloudBackup::FileUtil fu(filename);
  //std::cout << fu.FileName() << std::endl;
  //std::cout << fu.FileSize() << std::endl;
  //std::cout << fu.LastModifyTime() << std::endl;
  //std::cout << fu.LastAccessTime() << std::endl;

  //测试读取文件/写入文件
  //CloudBackup::FileUtil fu(filename);
  //std::string content;
  //fu.GetContent(content);
  //CloudBackup::FileUtil fu2("./test_Util.txt");
  //fu2.SetContent(content);

  //测试压缩与解压缩
  //std::string packname = filename + ".zp";
  //CloudBackup::FileUtil fu(filename);
  //fu.Compress(packname);
  //CloudBackup::FileUtil unfu(packname);
  //fu.UnCompress(filename + "-bak");

  //测试目录的创建, 遍历目录下的所有文件(打印它们的相对路径)
  //CloudBackup::FileUtil fu(filename);
  //fu.CreateDirectory();
  //std::vector<std::string> array;
  //fu.ScanDirectory(array);
  //for(auto& p : array){
  //  std::cout << p << std::endl;
  //}
}

int main(int argc, char* argv[])
{
  if(argc != 2){
    std::cerr << "Plz input 2 arguments!" << std::endl;
    return -1;
  }
  //std::string filename = "README.md";
  std::string filename = argv[1];
  test_Util(filename);
  return 0;
}
