#pragma once
#include <unordered_map>
#include "httplib.h"
#include <Windows.h>
#include <process.h>
#include "Util.hpp"
constexpr auto PERSISTENCE_FILE_DATA = "\\cloud.dat";

//Tag: filename-fsize-LastModifyTime

//客户端只使用一个线程来管理指定目录下的文件，因此这里对于数据管理模块而言，可以不使用锁
namespace CloudBackup {
	class DataManager {
	public:
		DataManager(const std::string& backdir)
			:_persistence_file(backdir + PERSISTENCE_FILE_DATA)
		{
			//InitializeSRWLock(&_srw);
			InitLoad();
		}

		~DataManager()
		{
			Storage();
		}

		bool InitLoad() {
			FileUtil fu(_persistence_file);
			//注意! 这里要判断一下文件是否存在, 文件不存在的话直接return true
			if (fu.Exists() == false) {
				return true;
			}

			std::string content;
			fu.GetContent(content);

			auto arr = Split(content, "\n");
			for (auto& str : arr)
			{
				auto line = Split(str, "?");
				_path2Tag[line[0]] = line[1];
			}
			return true;
		}
		
		bool Storage(){
			std::stringstream ss;
			for (auto& p : _path2Tag)
			{
				ss << p.first << "?" << p.second << "\n";	//我们在这里人为定义key?value的切割符为冒号":"、不同的文件之间使用\n间隔
			}
			FileUtil fu(_persistence_file);
			fu.SetContent(ss.str());
			return true;
		}

		bool Insert(const std::string& key, const std::string& val) {
			//AcquireSRWLockExclusive(&_srw);
			_path2Tag[key] = val;
			//ReleaseSRWLockExclusive(&_srw);
			Storage();
			return true;
		}

		bool Update(const std::string& key, const std::string& val) {
			//AcquireSRWLockExclusive(&_srw);
			_path2Tag[key] = val;
			//ReleaseSRWLockExclusive(&_srw);
			Storage();
			return true;
		}

		bool GetOneByKey(const std::string& key, std::string& output_val) {
			//AcquireSRWLockShared(&_srw);
			auto it = _path2Tag.find(key);
			if (it == _path2Tag.end()) {
				std::cerr << "DataManager: GetOneByKey: find key failed!" << std::endl;
				//ReleaseSRWLockShared(&_srw);
				return false;
			}
			//ReleaseSRWLockShared(&_srw);

			output_val = it->second;
			return true;
		}

		static std::vector<std::string> Split(const std::string& s, const std::string& sep) {
			std::vector<std::string> res;
			size_t pos = 0, prev = 0;
			while ((pos = s.find(sep, prev)) != std::string::npos)
			{
				//出现了2个sep连在了一起, 此时跳过
				if (prev == pos) {
					prev = pos + sep.size();	//别忘了修改prev的值
					continue;
				}
				res.emplace_back(s.substr(prev, pos - prev));
				prev = pos + sep.size();	//这里是+sep.size(), 因为sep可能不是一个字符
			}

			//由于我们的传入参数s是const属性, 所以我们没办法通过在s的最后+=sep的方法来确保在上面的while循环一定能够切取到最后一部分字符串
			//因此，我们这里通过判断prev的下标来保证获取到最后一部分的数据。(如12/23/34, 此时34就只能在while循环外面处理了)
			if (prev < s.size())
			{
				res.emplace_back(s.substr(prev));
			}
			return res;
		}

	private:
		//SRWLOCK _srw;	//Windows下的读写锁
		std::string _persistence_file = PERSISTENCE_FILE_DATA;	//持久化存储文件(备份信息存储文件)
		std::unordered_map<std::string, std::string> _path2Tag;	//文件路径 ? 文件唯一标识符Tag
	};
}