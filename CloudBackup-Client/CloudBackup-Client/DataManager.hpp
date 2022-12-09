#pragma once
#include <unordered_map>
#include "Util.hpp"
constexpr auto PERSISTENCE_FILE_DATA = "./cloud.dat";

//Tag: filename-fsize-LastModifyTime

//�ͻ���ֻʹ��һ���߳�������ָ��Ŀ¼�µ��ļ����������������ݹ���ģ����ԣ����Բ�ʹ����
namespace CloudBackup {
	class DataManager {
	public:
		DataManager()
			:_persistence_file(PERSISTENCE_FILE_DATA)
		{
			InitLoad();
		}

		bool InitLoad() {
			FileUtil fu(_persistence_file);
			//ע��! ����Ҫ�ж�һ���ļ��Ƿ����, �ļ������ڵĻ�ֱ��return true
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
				ss << p.first << "?" << p.second << "\n";	//������������Ϊ����key?value���и��Ϊð��":"����ͬ���ļ�֮��ʹ��\n���
			}
			FileUtil fu(_persistence_file);
			fu.SetContent(ss.str());
			return true;
		}

		bool Insert(const std::string& key, const std::string& val) {
			_path2Tag[key] = val;
			Storage();
			return true;
		}

		bool Update(const std::string& key, const std::string& val) {
			_path2Tag[key] = val;
			Storage();
			return true;
		}

		bool GetOneByKey(const std::string& key, std::string& output_val) {
			auto it = _path2Tag.find(key);
			if (it == _path2Tag.end()) {
				std::cerr << "DataManager: GetOneByKey: find key failed!" << std::endl;
				return false;
			}

			output_val = it->second;
			return true;
		}

	private:
		std::vector<std::string> Split(const std::string& s, const std::string& sep) {
			std::vector<std::string> res;
			size_t pos = 0, prev = 0;
			while ((pos = s.find(sep, prev)) != std::string::npos)
			{
				//������2��sep������һ��, ��ʱ����
				if (prev == pos) {
					prev = pos + sep.size();	//�������޸�prev��ֵ
					continue;
				}
				res.emplace_back(s.substr(prev, pos - prev));
				prev = pos + sep.size();	//������+sep.size(), ��Ϊsep���ܲ���һ���ַ�
			}

			//�������ǵĴ������s��const����, ��������û�취ͨ����s�����+=sep�ķ�����ȷ���������whileѭ��һ���ܹ���ȡ�����һ�����ַ���
			//��ˣ���������ͨ���ж�prev���±�����֤��ȡ�����һ���ֵ����ݡ�(��12/23/34, ��ʱ34��ֻ����whileѭ�����洦����)
			if (prev < s.size())
			{
				res.emplace_back(s.substr(prev));
			}
			return res;
		}

	private:
		std::string _persistence_file = PERSISTENCE_FILE_DATA;	//�־û��洢�ļ�(������Ϣ�洢�ļ�)
		std::unordered_map<std::string, std::string> _path2Tag;	//�ļ�·�� : �ļ�Ψһ��ʶ��Tag
	};
}