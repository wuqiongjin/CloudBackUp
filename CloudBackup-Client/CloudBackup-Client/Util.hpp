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
				return 0;	//����0��ʾ�ļ�������
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
			//// ../abc/test.txt  ��ȡ������test.txt
			//auto pos = _filename.rfind('\\');	//ע��, �����ҵ�����'\', Linux�µ�Ŀ¼�ָ�����Windows�²�ͬ!
			//if (pos == std::string::npos) {
			//	return _filename;
			//}
			//return _filename.substr(pos + 1);
			return fs::path(_filename).filename().string();	//C++17���ļ�ϵͳ��Ľӿ��ǿ�ƽ̨��
		}

		bool GetPosLenContent(std::string& output_content, size_t pos, size_t len) {
			size_t fz = this->FileSize();
			if (pos + len > fz) {
				std::cerr << "Read Size exceed FileSize!" << std::endl;
				return false;
			}

			output_content.resize(len - pos, '\0');  //!!! �������Ҫ��resize, ��Ȼ����read��ʱ��û���޸�string

			std::ifstream ifs(_filename, std::ios::binary);  //ifsĬ�϶��ļ� (fstream��Ҫָ����д)
			if (ifs.is_open() == false) {
				std::cerr << "open ReadFile failed!" << std::endl;
				return false;
			}
			ifs.seekg(pos, std::ios::beg);  //��beg������ƫ��pos����
			ifs.read(&output_content[0], len);
			ifs.close();
			return true;
		}

		//��ȡ��ǰ�ౣ���filename��ȫ������
		bool GetContent(std::string& output_content) {
			return this->GetPosLenContent(output_content, 0, this->FileSize());
		}

		//д�뵽��ǰ���������filename�ļ���, д���������content
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

		//!�����������������"Ŀ¼"�ļ���صĲ�������!
//�ж�filename���Ŀ¼�Ƿ����
		bool Exists() {
			return fs::exists(_filename);
		}

		//��filenameĿ¼������ʱ, ������Ŀ¼
		bool CreateDirectory() {
			//if(this->Exists()){
			//  return true;
			//}
			return fs::create_directory(_filename); //�����Զ��ж�, ���Ŀ¼�Ѿ�����, �������κδ���; Ŀ¼�������򴴽�Ŀ¼
		}

		//����filenameĿ¼�µ������ļ�, ���ļ��洢��array������(����Ͳ���)
		//ע��: ���뵽array���ļ�������'���·��', ���ǲ���ֻ���ļ�������, ���ǻ���Ҫ������ǰ׺·��(��Ե�)
		bool ScanDirectory(std::vector<std::string>& output_array) {
			//��չ: ����ʹ��recursive_directory_iterator���Եݹ������filenameĿ¼�µ������ļ�(��������Ŀ¼���ļ�), ��ô�Ƿ���԰�Ŀ¼Ҳ�ϴ�����������?
			for (auto& p : fs::directory_iterator(_filename)) {
				output_array.emplace_back(fs::path(p).string());



				////�ж��Ƿ���Ŀ¼�ļ�, ����Ŀ¼�ļ���ӵ�array������
				//if (fs::is_directory(p) == false) {
				//	//�ж��û������·�������·�����Ǿ���·��(�����·���;���·������ͬ�Ĵ���)
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