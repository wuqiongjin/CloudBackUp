#include "Util.hpp"
#include "DataManager.hpp"
#include "Backup.hpp"

void test_recursive_directory()
{
	std::string dir = ".\\backupDir";
	std::cout << fs::current_path() << std::endl;
	for (auto p = fs::recursive_directory_iterator(dir); p != fs::recursive_directory_iterator(); ++p)
	{
		std::cout << p->path().string() << std::endl;
	}
	std::cout << "\n";
}

void test_Util()
{
	//测试Scan目录
	//CloudBackup::FileUtil fu("./");
	//std::vector<std::string> files;
	//fu.ScanDirectory(files);
	//for (auto& str : files)
	//{
	//	std::cout << str << std::endl;
	//}

	//测试递归Scan+生成目录结构文件
	//CloudBackup::FileUtil fu(R"(E:\C++学习\GitHub\CloudBackup\CloudBackup-Client\CloudBackup-Client\backupDir)");
	//fu.GenerateStructureFile();


	//CloudBackup::FileUtil fu(".\\backupDir");
	//std::vector<std::string> files;
	//fu.ScanDirectory(files);

	//测试: 调整用户的路径(相对路径和绝对路径的统一化处理)
	//fs::current_path(fs::path(R"(./backupDir)"));
	//auto res = fs::current_path();
	//std::cout << res << std::endl;

	//fs::current_path(fs::path(R"(./backupDir)"));
	//auto res = fs::current_path();
	//auto absolute = res.string();
	//int pos = absolute.rfind("\\");
	//auto str = absolute.substr(pos + 1);
	//std::cout << str << std::endl;

	//std::cout << "root_path: " << res.root_path() << std::endl;
	//std::cout << "root_directory: " << res.root_directory() << std::endl;
	//std::cout << "root_name: " << res.root_name() << std::endl;
	//std::cout << "relative_path: "<< res.relative_path() << std::endl;
	//std::cout << "parent_path: "<< res.parent_path() << std::endl;

	//test_recursive_directory();
}



//void test_Split()
//{
//	std::string s = "123//456//7/8//9/";
//	//auto ret = Split(s, "/");
//	for (auto& str : ret)
//	{
//		std::cout << str << std::endl;
//	}
//}

void test_DataManager()
{
	//1. 先录入数据
	//CloudBackup::FileUtil fu("./");
	//std::vector<std::string> files;
	//fu.ScanDirectory(files);

	//CloudBackup::DataManager dm;
	//for (auto& file : files)
	//{
	//	dm.Insert(file, file + "-" + std::to_string(file.size()));
	//}

	//2. 后读取数据
	//CloudBackup::DataManager dm;
	//std::string val;
	//dm.GetOneByKey(R"(.\cloud.dat)", val);
	//std::cout << val << std::endl;
}

int main()
{
	//test_Util();
	//test_Split();
	//test_DataManager();

	//struct stat st;
	//auto ret = stat(R"(E:\C++学习\GitHub\CloudBackup\CloudBackup-Client\CloudBackup-Client\中文\test.txt)", &st);
	//if (ret < 0) {
	//	std::cerr << "file not exists" << std::endl;
	//	return 0;
	//}
	//

	CloudBackup::Backup bp(R"(E:\C++学习\GitHub\CloudBackup\CloudBackup-Client\CloudBackup-Client\compress_test)");
	bp.RunModule();
	//test_Util();

	return 0;
}