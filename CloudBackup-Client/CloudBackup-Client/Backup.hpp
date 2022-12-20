#pragma once
#include "DataManager.hpp"
#include "httplib.h"
#include <Windows.h>

namespace CloudBackup {
	constexpr auto SERVER_IP = "152.136.211.148";
	constexpr auto SERVER_PORT = 2333;
	//文件备份和上传模块
	//1. 遍历指定文件夹下的目录
	//2. 判断文件是否需要备份(新的文件/修改的文件) --- 判断方法(根据Tag)
	//3. 将需要备份的文件上传, 并更改文件的备份信息到(哈希表中)
	class Backup {
	public:
		Backup(const std::string& backdir)
			:_back_dir(backdir),
			 _datam(new DataManager(backdir))
		{}
	private:
		bool Upload(const std::string& relative_filename, const std::string& absolute_filename) {
			httplib::Client cl(SERVER_IP, SERVER_PORT);
			FileUtil fu(absolute_filename);	//下面要进行文件的读取, 因此这里必须是绝对路径, 不然找不到对应的文件
			std::string content;
			fu.GetContent(content);

			httplib::MultipartFormData item;
			std::regex r("\\\\");
			std::string upload_filename = std::regex_replace(relative_filename, r, "/");	//上传文件名必须是"相对路径"
			upload_filename.erase(0, 2);	//服务端那边处理了backupDir的路径,备份目录为'./backupDir/', 因此客户端文件名不需要携带'/'前缀, 这里删除长度为2
				//有一说一, 这里不删应该也没问题。服务端那边拼接的路径即便是"./backupDir/./myWork/a.txt" 这样的, 也是能够正常打开的, 只不过"./"没意义而已
			item.filename = upload_filename;
			item.name = "file";	//服务端指定的name就是file
			item.content = content;
			item.content_type = "application/octet-stream";

			httplib::MultipartFormDataItems items;
			items.emplace_back(item);

			auto ret = cl.Post("/upload", items);	//ret是一个Result类型的结构体
			if (!ret || ret->status != 200) {
				std::cerr << "Backup: Upload: upload file failed!" << std::endl;
				return false;
			}
			return true;
		}

	public:
		bool RunModule() {
			while (1) {
				int enterCount = 0;
				//1. 遍历指定文件夹下的目录	(在次之前, 客户端那边一定要保证 传过来的备份路径是一个目录, 而不是文件)
				//files数组是一个pair数组. 第一个元素表示"相对路径"; 第二个元素表示"绝对路径"
				std::vector<std::pair<std::string, std::string>> files;
				FileUtil fu(_back_dir);
				fu.ScanDirectory(files);

				for (auto& file : files) {
					//2. 判断文件是否需要备份(新的文件/修改的文件) --- 判断方法(根据Tag)
					if (IsNeedBackup(file.second)) {
						enterCount++;
						//3. 将需要备份的文件上传;
						auto ret = Upload(file.first, file.second);
						if (ret == true) {
							//3. 更改文件的备份信息到(哈希表中)
							_datam->Insert(file.second, GetFileTag(file.second));
						}
					}
				}
				if (enterCount <= 1) {
					break;
				}
				Sleep(10);	//每次休息10毫秒ms
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
			// cloud.dat文件不需要提交
			if (filename.substr(filename.rfind("\\") + 1) == "cloud.dat") {
				return false;
			}

			std::string old_tag;

			auto ret = _datam->GetOneByKey(filename, old_tag);
			//1. 当文件存在且old_tag与现有的tag相同的时候, 才不需要备份
			if (ret && old_tag == GetFileTag(filename)) {
				return false;
			}

			//2. 当文件不存在 or old_tag != 现有的tag, 才考虑备份
			//	这里考虑一种情况, 如果一个文件从别的文件夹拷贝到backDir, 但是它很大, 拷贝的速度很慢。
			//	->	此时我们在检测的时候，会反复检测该文件是被修改过的，反复备份并上传该文件。
			//	->	为了防止这种情况的发生, 我们可以通过判断 当前时间 - 最后修改时间 < 3s, 来判断该文件可能还在被修改或者正在拷贝中。
			//	->	这种情况我们就返回false, 等待它超过这个时间后, 再备份该文件
			//if (time(NULL) - FileUtil(filename).LastModifyTime() < 3) {
			//	return false;
			//}

			return true;
		}

	private:
		std::string _back_dir;	//监控的文件目录(备份文件夹)
		DataManager* _datam;	//数据管理模块
	};
}