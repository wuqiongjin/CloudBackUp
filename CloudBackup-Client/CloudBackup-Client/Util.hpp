#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <ctime>
#include <experimental/filesystem>
//#include <jsoncpp/json/json.h>
#include <sys/stat.h>
//#include "./liba/include/bundle.h"
namespace fs = std::experimental::filesystem;

namespace CloudBackup {
	class FileUtil {
	public:
		FileUtil(const std::string& name) 
			:_filename(name),
			 _is_absolute_path(fs::path(name).is_absolute())
		{}

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

		//!下面三个函数是针对"目录"文件相关的操作函数!
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
		//注意: 存入到array的文件名称是'相对路径', 我们不是只存文件的名字, 我们还需要存它的前缀路径(相对的)
		bool ScanDirectory(std::vector<std::string>& output_array) {
			//扩展: 这里使用recursive_directory_iterator可以递归遍历到filename目录下的所有文件(包含其子目录的文件), 那么是否可以把目录也上传到服务器呢?
			for (auto& p : fs::directory_iterator(_filename)) {
				output_array.emplace_back(fs::path(p).string());



				////判断是否是目录文件, 将非目录文件添加到array数组中
				//if (fs::is_directory(p) == false) {
				//	//判断用户输入的路径是相对路径还是绝对路径(对相对路径和绝对路径作不同的处理)
				//	if (_is_absolute_path) {
				//		output_array.emplace_back(fs::path(p).string());
				//	}
				//	else {
				//		output_array.emplace_back(fs::path(p).relative_path().string());
				//	}
				//}
			}
			return true;
		}

	private:
		std::string _filename;
		struct stat _st;
		bool _is_absolute_path = true;
	};
}