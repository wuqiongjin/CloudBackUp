#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <utility>
#include <regex>
#include <ctime>
#include <experimental/filesystem>
//#include <jsoncpp/json/json.h>
#include <sys/stat.h>
//#include "./liba/include/bundle.h"
namespace fs = std::experimental::filesystem;

namespace CloudBackup {
	//文件实用工具类:
	//这个类既能够给普通文件使用, 也能够给目录文件使用
	class FileUtil {
	public:
		FileUtil(const std::string& name) 
			:_filename(name)
		{
			//统一目录名称(实际上就是 获取绝对路径, 以便于后续处理)
			if (fs::is_directory(name)) {
				UnifyDirectoryName(name);
			}
			//普通文件不处理
		}

		const std::string& GetAbsolutePath() { return _absolute_path; }
		const std::string& GetRelativePath() { return _relative_path; }

		bool RemoveFile(const std::string& filename) {
			return fs::remove(filename);	//不管它是否存在, 都会进行删除; 不存在false, 存在true
		}

		size_t FileSize() {
			if (stat(_filename.c_str(), &_st) < 0) {
				std::cerr << "Get FileSize Failed!" << std::endl;
				return 0;	//返回0表示文件不存在
			}
			return _st.st_size;
		}

		time_t LastModifyTime() {
			if (stat(_filename.c_str(), &_st) < 0) {
				std::cerr << "Get LastModifyTime Failed!" << std::endl;
				return -1;
			}
			return _st.st_mtime;
		}

		time_t LastAccessTime() {
			if (stat(_filename.c_str(), &_st) < 0) {
				std::cerr << "Get LastAccessTime Failed!" << std::endl;
				return -1;
			}
			return _st.st_atime;
		}

		std::string FileName() {
			//// ../abc/test.txt  获取到最后的test.txt
			//auto pos = _filename.rfind('\\');	//注意, 这里找到的是'\', Linux下的目录分隔符和Windows下不同!
			//if (pos == std::string::npos) {
			//	return _filename;
			//}
			//return _filename.substr(pos + 1);
			return fs::path(_filename).filename().string();	//C++17的文件系统库的接口是跨平台的
		}

		bool GetPosLenContent(std::string& output_content, size_t pos, size_t len) {
			size_t fz = this->FileSize();
			if (pos + len > fz) {
				std::cerr << "Read Size exceed FileSize!" << std::endl;
				return false;
			}

			output_content.resize(len - pos, '\0');  //!!! 这里必须要先resize, 不然后面read的时候没法修改string

			std::ifstream ifs(_filename, std::ios::binary);  //ifs默认读文件 (fstream需要指定读写)
			if (ifs.is_open() == false) {
				std::cerr << "open ReadFile failed!" << std::endl;
				return false;
			}
			ifs.seekg(pos, std::ios::beg);  //从beg处往后偏移pos长度
			ifs.read(&output_content[0], len);
			ifs.close();
			return true;
		}

		//读取当前类保存的filename的全部内容
		bool GetContent(std::string& output_content) {
			return this->GetPosLenContent(output_content, 0, this->FileSize());
		}

		//写入到当前类所保存的filename文件中, 写入的内容是content
		bool SetContent(const std::string& content) {
			std::ofstream ofs(_filename, std::ios::binary);
			if (ofs.is_open() == false) {
				std::cerr << "open WriteFile failed!" << std::endl;
				return false;
			}
			ofs.write(&content[0], content.size());
			ofs.close();
			return true;
		}

		//!下面三个函数是针对"目录"文件相关的操作函数! (Exists、CreateDriectory、ScanDirectory)
//判断filename这个目录是否存在
		bool Exists() {
			return fs::exists(_filename);
		}

		//当filename目录不存在时, 创建该目录
		bool CreateDirectory() {
			//if(this->Exists()){
			//  return true;
			//}
			return fs::create_directory(_filename); //它会自动判断, 如果目录已经存在, 不会做任何处理; 目录不存在则创建目录
		}

		//遍历filename目录下的所有文件, 将文件存储到array数组中(输出型参数)
		//注意: array的pair的第一个表示"相对路径"; 第二个表示"绝对路径";
		//相对路径是用于在服务端'创建'文件并'写入'的(以及'创建目录结构'); 绝对路径是用于在客户端'读取'文件内容的
		bool ScanDirectory(std::vector<std::pair<std::string, std::string>>& output_array) {
			std::string file_tree = _absolute_path + "\\" + FileName() + "_dir.stru";
			RemoveFile(file_tree);	//提前清除目录结构文件(为了保证它是第一个数组元素)
			output_array.emplace_back(ConvertAbsolute2Relative(file_tree), file_tree);	//保证第一个元素一定是目录结构文件
			for (auto& p : fs::recursive_directory_iterator(_absolute_path)) {
				if (fs::is_directory(p) == false) {
					output_array.emplace_back(ConvertAbsolute2Relative(fs::path(p).string()), fs::path(p).string());
				}
			}

			//最后生成一下 目录结构文件
			return GenerateStructureFile();
		}

	private:	//下面的几个函数只会在当前类中被调用
		//返回生成的目录结构文件名称
		bool GenerateStructureFile() {
			std::string res;
			std::string line;
			std::regex r("\\\\");	//将\\替换为/
			for (auto p = fs::recursive_directory_iterator(_absolute_path); p != fs::recursive_directory_iterator(); ++p)
			{
				if (fs::is_directory(p->path())) {
					line = p->path().string();
					line = ConvertAbsolute2Relative(line);	//将绝对路径转为为相对路径
					line = std::regex_replace(line, r, "/");
					line.erase(0, 2);	//这边删不删都不影响服务端那边创建目录
					res += line + "\n";
				}
			}
			if (res != "") {
				res.resize(res.size() - 1);	//删除最后多加的'\n'
			}
			std::string file_tree = _absolute_path + "\\" + FileName() + "_dir.stru";
			FileUtil fu(file_tree);	//备份根目录下会生成xxx_dir.stru目录结构文件
			fu.SetContent(res);
			return true;
		}

		//统一化目录名称 ---> 保存绝对路径(后续在上传文件时会修改为相对路径)
		bool UnifyDirectoryName(const std::string& name){
			//原始工作目录
			std::string original_work_dir = fs::current_path().string();

			fs::current_path(name);						//改变当前工作目录到name目录下

			_absolute_path = fs::current_path().string();	//获取当前工作目录的绝对路径
			//int pos = _absolute_path.rfind("\\");		//找到备份的目录名称
			//_relative_path = ".\\";
			//_relative_path += _absolute_path.substr(pos + 1);	//获取相对路径	.\\backupDir\\xxx的形式

			fs::current_path(original_work_dir);		//回到原始工作目录(以防影响后续使用FileUtil类)
			return true;
		}

		//功能: 将绝对路径转换成相对路径。相对于"备份根目录的"相对路径! (这里的路径分隔符是"\\" Windows的)
		//注: 调用该函数的有ScanDirectory、GenerateStructureFile.
		//示例:
		//备份目录:	E:\\WorkFlow\\DailyNotes\\BackupDir
		//处理对象:	E:\\WorkFlow\\DailyNotes\\BackupDir\\xxx
		//处理结果:	.\\BackupDir\\xxx
		inline std::string ConvertAbsolute2Relative(const std::string& path) {
			std::string res = ".\\";
			res += path.substr(_absolute_path.size() - FileName().size());
			return res;
		}

	private:
		std::string _filename;
		struct stat _st;
		std::string _absolute_path;
		std::string _relative_path;
	};
}