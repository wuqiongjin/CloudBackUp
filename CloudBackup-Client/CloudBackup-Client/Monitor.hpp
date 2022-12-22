#pragma once
#include "Backup.hpp"

namespace CloudBackup {
	//���ģ���ִ��ʱֻ�����̲߳�����. ���߳�����ִ�б���ģ���. ��˼��ģ�鲻��Ҫ����
	constexpr auto MONITOR_DAT = "\\MonitoredBackupList.dat";	//����ļ��б�����exe�����ͬһ������
	constexpr auto MAX_STOPS = 1000;	//�������1000����� (������ʼ����ӵļ�ر���Ŀ¼)
	class Monitor {
	public:
		Monitor()
			:_fu(MONITOR_DAT),
			 _stops(MAX_STOPS, false)
		{
			_fu = FileUtil(fs::current_path().string() + MONITOR_DAT);
			InitLoad();
		}

		~Monitor() 
		{
			Storage();
			//�������߳̽�����־λ������Ϊtrue
			for (auto& ret : _monitor_list)
			{
				_stops[ret.second] = true;
			}
		}

	private:
		bool InitLoad() {
			if (_fu.Exists() == false) {
				return true;
			}

			std::string data;
			_fu.GetContent(data);
			auto ret = DataManager::Split(data, "\n");
			for (auto& path : ret)
			{
				_monitor_list.emplace(path, AddThread(path));
			}
			return true;
		}

		bool Storage() {
			std::string content;
			for (auto& ret : _monitor_list)
			{
				//ֻ�־û��洢����б�
				if (_stops[ret.second] == false) {
					content += ret.first + '\n';
				}
			}
			_fu.SetContent(content);
			return true;
		}

	public:
		bool Insert(const std::string& path) {
			_monitor_list.emplace(path, AddThread(path));	//�����߳�, ��������״̬Ϊ ���״̬
			//AddThread(path);
			Storage();
			return true;
		}

		bool Delete(const std::string& path) {
			if (_monitor_list.find(path) == _monitor_list.end()) {
				return true;
			}
			_stops[_monitor_list[path]] = true;	//����Ϊ �Ǽ��״̬ (����Ӽ���б���ɾ��), ������߳̾ͻ��RunModue������, Ȼ����ֹ
			_monitor_list.erase(path);
			Storage();
			return true;
		}

		bool Search(const std::string& path) {
			return _monitor_list.find(path) != _monitor_list.end();
		}

		std::vector<std::string> ShowMonitorList() {
			std::vector<std::string> res;
			for (auto& ret : _monitor_list)
			{
				//������'�����״̬'��Ŀ¼��ӽ���
				if (_stops[ret.second] == false) {
					res.emplace_back(ret.first);
				}
			}
			return res;
		}

	private:
		//�̵߳�ִ�к���
		static void RealTimeBackup(const std::string& path, const std::vector<bool>& stops, int index) {
			Backup bp(path);
			//const bool& stop_flag = stops[index];	//����������
			bp.RunModule(stops, index);
		}

		//��������ӵ��߳����ڵ������±� (���һ��λ�õ��±�)
		int AddThread(const std::string& path) {
			//_stops.emplace_back(false);
			_ths.emplace_back(std::thread(RealTimeBackup, std::ref(path), std::ref(_stops), _ths.size()));
			_ths.back().detach();
			return _ths.size() - 1;
		}

	private:
		FileUtil _fu;
		std::vector<std::thread> _ths;
		std::vector<bool> _stops;			//ֹͣ��־, ��stop����Ϊtrueʱ, ֹͣ��ظ��±�����Ӧ���߳�
		std::unordered_map<std::string, int> _monitor_list;//����Ŀ¼ to �߳������±�
	};
}