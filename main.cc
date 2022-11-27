#include "Util.hpp"
#include "Config.hpp"
#include "DataManager.hpp"
#include "HotManager.hpp"

CloudBackup::DataManager* _datam;

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

void test_DataManager(const std::string& filename)
{
  //BackupInfo - Test
  //CloudBackup::BackupInfo bi;
  //bi.NewBackupInfo(filename);
  //std::cout << bi._pack_flag << std::endl;
  //std::cout << bi._fsize << std::endl;
  //std::cout << bi._mtime << std::endl;
  //std::cout << bi._atime << std::endl;
  //std::cout << bi._real_path << std::endl;
  //std::cout << bi._pack_dir << std::endl;
  //std::cout << bi._url << std::endl;

  //DataManager - Test
  CloudBackup::DataManager dm;
  //CloudBackup::BackupInfo bi;
  //bi.NewBackupInfo(filename);
  //dm.Insert(bi);
  std::vector<CloudBackup::BackupInfo> array; 
  dm.GetAll(array);
  for(size_t i = 0; i < array.size(); ++i)
  {
    std::cout << array[i]._pack_flag << std::endl;
    std::cout << array[i]._fsize << std::endl;
    std::cout << array[i]._mtime << std::endl;
    std::cout << array[i]._atime << std::endl;
    std::cout << array[i]._real_path << std::endl;
    std::cout << array[i]._pack_dir << std::endl;
    std::cout << array[i]._url << std::endl;
  }
}

void test_HotManager()
{
  _datam = new CloudBackup::DataManager();
  CloudBackup::HotManager ht;
  ht.RunModule();
}

int main(int argc, char* argv[])
{
  //if(argc != 2){
  //  std::cerr << "Plz input 2 arguments!" << std::endl;
  //  return -1;
  //}
  //std::string filename = "README.md";
  //std::string filename = argv[1];
  
  //test_Util(filename);
  //test_DataManager(filename);
  test_HotManager();
  return 0;
}
