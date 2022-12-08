#pragma once
#include "DataManager.hpp"
#include "httplib.h"
#include <Windows.h>

namespace CloudBackup {
	constexpr auto SERVER_IP = "152.136.211.148";
	constexpr auto SERVER_PORT = 2333;
	//�ļ����ݺ��ϴ�ģ��
	//1. ����ָ���ļ����µ�Ŀ¼
	//2. �ж��ļ��Ƿ���Ҫ����(�µ��ļ�/�޸ĵ��ļ�) --- �жϷ���(����Tag)
	//3. ����Ҫ���ݵ��ļ��ϴ�, �������ļ��ı�����Ϣ��(��ϣ����)
	class Backup {
	public:
		Backup(const std::string& backdir)
			:_back_dir(backdir),
			 _datam(new DataManager())
		{}
	private:
		bool Upload(const std::string& filename) {
			httplib::Client cl(SERVER_IP, SERVER_PORT);
			FileUtil fu(filename);
			std::string content;
			fu.GetContent(content);

			httplib::MultipartFormData item;
			//item.filename = fu.FileName();
			std::regex r("\\\\");
			std::string upload_filename = std::regex_replace(filename, r, "/");
			upload_filename.erase(0, 2);	//������Ǳߴ�����backupDir��·��,����Ŀ¼Ϊ'./backupDir/', ��˿ͻ����ļ�������ҪЯ��'/'ǰ׺, ����ɾ������Ϊ2
				//��һ˵һ, ���ﲻɾӦ��Ҳû���⡣������Ǳ�ƴ�ӵ�·��������"./backupDir/./myWork/a.txt" ������, Ҳ���ܹ������򿪵�, ֻ����"./"û�������
			item.filename = upload_filename;
			item.name = "file";	//�����ָ����name����file
			item.content = content;
			item.content_type = "application/octet-stream";

			httplib::MultipartFormDataItems items;
			items.emplace_back(item);

			auto ret = cl.Post("/upload", items);	//ret��һ��Result���͵Ľṹ��
			if (!ret || ret->status != 200) {
				std::cerr << "Backup: Upload: upload file failed!" << std::endl;
				return false;
			}
			return true;
		}

	public:
		bool RunModule() {
			while (1) {
				bool enter = false;
				//1. ����ָ���ļ����µ�Ŀ¼	(�ڴ�֮ǰ, �ͻ����Ǳ�һ��Ҫ��֤ �������ı���·����һ��Ŀ¼, �������ļ�)
				std::vector<std::string> files;
				FileUtil fu(_back_dir);
				fu.ScanDirectory(files);

				for (auto& file : files) {
					//2. �ж��ļ��Ƿ���Ҫ����(�µ��ļ�/�޸ĵ��ļ�) --- �жϷ���(����Tag)
					if (IsNeedBackup(file)) {
						enter = true;
						//3. ����Ҫ���ݵ��ļ��ϴ�;
						auto ret = Upload(file);
						if (ret == true) {
							//3. �����ļ��ı�����Ϣ��(��ϣ����)
							_datam->Insert(file, GetFileTag(file));
						}
					}
				}
				if (!enter) {
					break;
				}
				Sleep(10);	//ÿ����Ϣ10����ms
			}
			
			return true;
		}


	private:
		std::string GetFileTag(const std::string& filename) {
			FileUtil fu(filename);
			std::stringstream ss;
			ss << filename << "-" << fu.FileSize() << "-" << fu.LastModifyTime();
			return ss.str();
		}

		bool IsNeedBackup(const std::string& filename) {
			std::string old_tag;

			auto ret = _datam->GetOneByKey(filename, old_tag);
			//1. ���ļ�������old_tag�����е�tag��ͬ��ʱ��, �Ų���Ҫ����
			if (ret && old_tag == GetFileTag(filename)) {
				return false;
			}

			//2. ���ļ������� or old_tag != ���е�tag, �ſ��Ǳ���
			//	���￼��һ�����, ���һ���ļ��ӱ���ļ��п�����backDir, �������ܴ�, �������ٶȺ�����
			//	->	��ʱ�����ڼ���ʱ�򣬻ᷴ�������ļ��Ǳ��޸Ĺ��ģ��������ݲ��ϴ����ļ���
			//	->	Ϊ�˷�ֹ��������ķ���, ���ǿ���ͨ���ж� ��ǰʱ�� - ����޸�ʱ�� < 3s, ���жϸ��ļ����ܻ��ڱ��޸Ļ������ڿ����С�
			//	->	����������Ǿͷ���false, �ȴ����������ʱ���, �ٱ��ݸ��ļ�
			//if (time(NULL) - FileUtil(filename).LastModifyTime() < 3) {
			//	return false;
			//}

			return true;
		}

	private:
		std::string _back_dir;	//��ص��ļ�Ŀ¼(�����ļ���)
		DataManager* _datam;	//���ݹ���ģ��
	};
}