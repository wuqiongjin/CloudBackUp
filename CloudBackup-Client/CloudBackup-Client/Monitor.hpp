#pragma once
#include "Backup.hpp"

namespace CloudBackup {
	//监控模块的执行时只有主线程操作的. 子线程们是执行备份模块的. 因此监控模块不需要加锁
	constexpr auto MONITOR_DAT = "\\MonitoredBackupList.dat";	//监控文件列表保存在exe程序的同一级别下
	constexpr auto MAX_STOPS = 1000;	//最多允许1000次添加 (包含初始化添加的监控备份目录)
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
			//将所有线程结束标志位都设置为true
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
				//只持久化存储监控列表
				if (_stops[ret.second] == false) {
					content += ret.first + '\n';
				}
			}
			_fu.SetContent(content);
			return true;
		}

	public:
		bool Insert(const std::string& path) {
			_monitor_list.emplace(path, AddThread(path));	//创建线程, 并设置其状态为 监控状态
			//AddThread(path);
			Storage();
			return true;
		}

		bool Delete(const std::string& path) {
			if (_monitor_list.find(path) == _monitor_list.end()) {
				return true;
			}
			_stops[_monitor_list[path]] = true;	//设置为 非监控状态 (将其从监控列表中删除), 随后子线程就会从RunModue中跳出, 然后终止
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
				//将处在'被监控状态'的目录添加进来
				if (_stops[ret.second] == false) {
					res.emplace_back(ret.first);
				}
			}
			return res;
		}

	private:
		//线程的执行函数
		static void RealTimeBackup(const std::string& path, const std::vector<bool>& stops, int index) {
			Backup bp(path);
			//const bool& stop_flag = stops[index];	//这样做不行
			bp.RunModule(stops, index);
		}

		//返回新添加的线程所在的数组下标 (最后一个位置的下标)
		int AddThread(const std::string& path) {
			//_stops.emplace_back(false);
			_ths.emplace_back(std::thread(RealTimeBackup, std::ref(path), std::ref(_stops), _ths.size()));
			_ths.back().detach();
			return _ths.size() - 1;
		}

	private:
		FileUtil _fu;
		std::vector<std::thread> _ths;
		std::vector<bool> _stops;			//停止标志, 当stop设置为true时, 停止监控该下标所对应的线程
		std::unordered_map<std::string, int> _monitor_list;//备份目录 to 线程所在下标
	};
}