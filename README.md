[toc]

# 文件云同步系统

## 1.项目功能

### 1.1 功能概述

&emsp;用户在客户端界面中输入想要备份的目录的路径，然后该目录下的所有文件会被逐个上传到服务器当中。服务器会接收来自客户端上传的备份文件，然后保存客户端原有的目录结构，并将所有的文件存放到服务器指定的目录下。服务端除了能够接收客户端的文件外。服务端还会进行热点管理，即，自动检测客户端上传的备份文件是否是非热点文件*(长时间没有被访问的文件就是非热点文件)*，然后将非热点文件自动进行压缩处理。用户可以通过浏览器访问服务器对应的IP地址和端口号，以此来查看用户已经备份的文件，并且支持普通文件的下载以及断点下载。

&emsp;

### 1.2 服务端具体功能

1. 接收客户端上传的文件, 并进行备份存储。

   > 借助httplib库实现网络通信, 编写对应的请求处理方法。
   >
   > ``svr.Post("/upload", Upload)``
   >
   > 判断请求文件是否为``xxx_dir.stru``, 如果是, 则创建对应的目录结构, 然后返回。
   >
   > 如果不是, 则进行文件的写入存储操作, 将文件存储到对应的路径下(文件名即是路径)。

2. 对备份文件进行热点管理, 对于长时间没有访问的非热点文件会进行压缩处理。

   > 服务端也存在记录文件信息的数据文件``cloud.dat``。
   >
   > 根据``cloud.dat``中的时间信息, 判断当前文件是否超出"活跃时间", 超出活跃时间则是非热点文件。此时进行压缩处理。
   >
   > 压缩方法使用``bundle``库进行压缩, 压缩格式采用``ZPAQ``格式。
   >
   > 考虑到客户端上传的文件数量可能很多, 因此服务端在压缩时使用线程池处理压缩任务。

3. 支持浏览器访问备份文件列表, 以及访问列表当中的目录文件。

   > &emsp;浏览器默认访问界面为根目录, 因此根目录资源请求处理为返回备份目录的根目录的文件列表信息。
   >
   > &emsp;对于访问目录文件, 扫描该目录下当前层的所有文件。将当前层的所有文件组织成文件信息列表返回给浏览器对应响应页面。

4. 支持浏览器下载备份文件, 并支持断点下载。

   > 下载文件, 设置对应``content-type``字段为``application/octet-stream``。文件对应``href``为文件相对备份目录根目录的路径。这样浏览器在点击对应文件的时候, 就可以触发下载请求了。服务器在处理下载任务时, 首先会判断该文件是否已被压缩, 将被压缩的文件还原至原始路径下, 然后再读取对应文件数据, 写入响应正文中。
   >
   > 断点下载, 服务端为每个文件资源生成对应的``ETag``标志, 规则为``filename-fsize-LastModifyTime``。
   >
   > 服务器在客户端第一次发送下载请求时, 就在响应报头中设置``ETag:xxx``。这样在后续进行断点下载时可以通过客户端的请求中是否含有请求报头``If-Range``即可。``httplib``库可以自动处理``If-Range``请求数据的区间, 服务端只需要将文件数据赋给响应正文即可。

&emsp;

### 1.3 客户端具体功能

1. Qt图形化界面

   > 重点在于2个button的功能。一个是Apply，另一个是Exit。ApplyButton会触发备份操作, 它会读取LineEdit中的内容, 然后将``QString``转化为``std::string``, 并解决编码的问题。随后将该字符串传入备份启动函数当中。ExitButton会退出程序, 在退出程序之前会触发默认备份操作。即, 将当前LineEdit输入框中的备份路径下的文件进行检测, 将需要备份的再次上传至服务器。

2. 检测用户指定的备份目录下的文件是否需要备份

   > 检测方法:
   >
   > &emsp;扫描备份目录下的所有普通文件, 将所有普通文件加入待备份数组中, 然后根据``cloud.dat``文件的文件信息判断该文件是否是新增的/被修改过的。如果是, 那么``Upload``它。
   >
   > ``cloud.dat``是什么?
   >
   > &emsp;每备份完一个文件时, 会生成/更新``cloud.dat``文件, 用于保存已经备份的文件信息。程序每次启动时, 会读取``cloud.dat``文件的数据到内存当中。
   >
   > 注意: ``cloud.dat``本身不会被上传至服务器中。

3. 将需要备份的文件逐个上传至服务器

   > 借助httplib库与服务端进行网络数据通信。

4. 支持备份路径下包含多层级子目录结构

   > 生成目录结构文件``xxx_dir.stru``。该文件中保存了备份目录下的目录结构。
   >
   > &emsp;在进行文件上传的时候, 优先上传目录结构文件给服务端。服务端根据目录结构文件在服务端备份目录中创建对应的目录结构。
   >
   > &emsp;随后, 在上传普通文件时, 由于普通文件的文件名称是它相对应备份目录的相对路径, 因此可以知道它在目录结构中的具体位置。在上传至服务器后, 也能够将其保存在对应的目录结构下。

&emsp;

额外支持特性:

1. 允许用户输入绝对路径或相对路径。

   > 对用户输入的路径进行统一化处理。*(将绝对路径和相对路径统一处理为相对路径, 以便于文件上传)*

2. 允许可执行程序(.exe)不与备份目录处于同一路径下。

   > 程序运行后, 在用户提交备份路径后会切换当前工作目录至备份路径, 随后会在备份目录下生成对应的目录结构文件.stru以及在生成备份数据信息文件cloud.dat

3. 用户点击Apply备份指定目录下的文件。

   > 扫描该目录下的所有文件。首先生成一个目录结构文件``xxx_dir.stru``。该文件保存了备份目录下的目录树结构(多层级目录结构)。随后再将所有普通文件(非目录文件)保存至文件待上传数组中, 依次上传至服务器(优先上传目录结构文件, 让服务端先根据目录结构文件生成对应的目录结构)。

&emsp;

&emsp;

## 2.服务端模块详解

### 2.1 文件实用工具类模块

&emsp;主要为文件和目录提供一些操作方法。其中``Exists()``、``CreateDirectory()``、``ScanDirectory()``是为目录提供的, 其余接口是为文件提供的。

```cpp
//文件实用工具类
class FileUtil{
public:
  FileUtil(const std::string &name): _filename(name){}
  bool RemoveFile();
  int64_t FileSize();
  time_t LastModifyTime();
  time_t LastAccessTime();
  std::string Time2String(time_t t);
  std::string FileName();
  bool GetPosLenContent(std::string &output_content, size_t pos, size_t len);//获取文件部分内容
  bool GetContent(std::string &output_content);//获取文件的所有内容
  bool SetContent(const std::string &content); //写文件
  bool Compress(const std::string &packname);  //压缩
  bool UnCompress(const std::string &filename);//解压缩
  bool Exists();
  bool CreateDirectory();
  bool ScanDirectory(std::vector<std::string> &output_array);
public:
  static std::vector<std::string> Split(const std::string &s, const std::string &sep);
private:
  std::string _filename;
  struct stat _st;
};
```

&emsp;

### 2.2 Json实用工具类模块

&emsp;Json实用工具类主要用于序列化和反序列化。

序列化: 将内存中的数据填入Json对象当中, 以便于序列化成Json格式的字符串, 最终存入磁盘文件中。

反序列化: 读取磁盘文件到内存中(Json格式字符串), 将Json格式字符串反序列化到Json对象中, 最终从Json对象中取出对应的值。

```cpp
class JsonUtil{
public:
  static bool Serialize(Json::Value &obj, std::string &output_JsonString);
  static bool Deserialize(std::string &JsonString, Json::Value &output_obj);
};
```

&emsp;

### 2.3 配置信息模块

&emsp;该模块主要管理服务器相关配置信息: 热点时间的设置、服务器IP、服务器端口号、压缩包后缀、压缩路径、备份文件路径、备份文件名称、下载请求URL前缀、访问请求URL前缀。(这些信息都存放于``Cloud.conf``文件当中, 方便修改)

**注意**: 该模块设计为单例模式。

```cpp
constexpr auto CONFIG_FILE = "./Cloud.conf";

//配置信息模块(单例类)
class Config{
public:
  static Config *GetInstance();
  int GetHotTime() { return _hot_time; }
  std::string GetServerIP() { return _server_ip; }
  int GetServerPort() { return _server_port; }
  std::string GetPackSuffix() { return _pack_suffix; }
  std::string GetPackDir() { return _pack_dir; }
  std::string GetBackupDir() { return _backup_dir; }
  std::string GetBackupInfoList() { return _backup_info_list; }
  std::string GetDownloadPrefix() { return _download_prefix; }
  std::string GetAccessPrefix() { return _access_prefix; }

private:
  bool ReadConfigFile(); //从CONFIG_FILE中读取配置文件信息
private:
  Config() { ReadConfigFile(); }
  Config(const Config &) = delete;
  Config& operator=(const Config &) = delete;
  static Config *_inst;
  static std::mutex _mtx;

private:
  int _hot_time;                 //热点时间, 超过这个时间没有访问, 进行压缩备份
  std::string _server_ip;        //服务器IP
  int _server_port;              //服务器端口号
  std::string _pack_suffix;      //压缩包后缀名
  std::string _pack_dir;         //压缩包的存放路径
  std::string _backup_dir;       //备份文件的存放路径
  std::string _backup_info_list; //备份文件数据信息列表(备份文件的信息存放到cloud.dat文件中)
  std::string _download_prefix;  //下载备份文件时需要为下载请求URL添加前缀以用于触发对应响应
  std::string _access_prefix;    //访问目录文件时需要为URL添加访问前缀以用于触发对应响应
};
Config *Config::_inst = nullptr;
std::mutex Config::_mtx;
```

&emsp;

**<font size="4">• 配置信息</font>**

&emsp;下面这些配置信息保存在``Cloud.conf``文件中。

```json
{
  "hot_time" : 30,
  "server_ip" : "0.0.0.0",
  "server_port" : 2333,
  "pack_suffix" : ".zp",
  "pack_dir" : "./packDir/",
  "backup_dir" : "./backupDir/",
  "backup_info_list" : "./cloud.dat",
  "download_prefix" : "/download/",
  "access_prefix" : "/access/"
}
```

&emsp;

### 2.4 数据管理模块

&emsp;该模块用于对备份文件的信息进行管理。管理包括: 插入备份文件信息、更新备份文件信息、获取备份信息、将备份文件信息持久化到磁盘文件中、初始化加载备份文件信息到哈希表中。``BackupInfo``类是用于描述备份文件信息的类。

**注意**: 该模块可以设计为单例模式。(我这里直接在main函数中定义了一个``DataManager``类的全局变量以用于访问)

```cpp
struct BackupInfo{
public:
  bool NewBackupInfo(const std::string &realpath);

public:
  bool _file_type = true;  // 文件类型: true为普通文件; false为目录文件
  bool _pack_flag = false; // 判断文件是否已经被压缩了
  size_t _fsize;           // 文件大小
  time_t _mtime;           // 文件最后一次修改时间
  time_t _atime;           // 文件最后一次访问时间
  std::string _real_path;  // 文件实际存放路径 (./backupDir/filename.xxx)
  std::string _pack_dir;   // 压缩包存放路径 (./packDir/filename.xxx.zp)
  std::string _url;        // 下载文件时的URL请求资源路径
};

class DataManager{
public:
  DataManager(){
    _persistence_file = Config::GetInstance()->GetBackupInfoList();
    pthread_rwlock_init(&_rwlock, nullptr);
    InitLoad();
  }
  ~DataManager() { pthread_rwlock_destroy(&_rwlock); }
  bool Insert(const BackupInfo &info);
  bool Update(const BackupInfo &info);
  bool GetOneByURL(const std::string &url, BackupInfo &output_info);
  bool GetOneByRealPath(const std::string &realpath, BackupInfo &output_info);
  bool GetCurrentAll(const std::string &cur_path_url, \
                     std::vector<BackupInfo> &output_array);
  bool GetAll(std::vector<BackupInfo> &output_array);
  bool Storage();
  bool InitLoad();

private:
  std::string _persistence_file;		//用于进行持久化存储文件的文件名称(备份信息文件)
  std::unordered_map<std::string, BackupInfo> _path2InfoTable; // URL路径 to 数据信息
  pthread_rwlock_t _rwlock;                                    // 读写锁
};
```

&emsp;

### 2.5 热点管理模块

&emsp;该模块的功能是判断文件是否为非热点文件, 如果是则将文件进行压缩。将压缩任务推送到线程池当中, 让线程池帮我们完成。压缩完毕后会删除原文件, 并设置压缩标志位为true(表示已压缩)。

**注意**: 备份目录下的文件在压缩后会被删除, 然后存放到压缩目录下。在后续解压时再还原到对应的备份路径中。

```cpp
extern CloudBackup::DataManager *_datam;

class HotManager{
public:
  HotManager()
      : _tp(new ThreadPool())
  {
    // 读取配置文件信息到 成员变量中
    Config *cf = Config::GetInstance();
    _hot_time = cf->GetHotTime();
    _back_dir = cf->GetBackupDir();
    _pack_dir = cf->GetPackDir();
    _pack_suffix = cf->GetPackSuffix();

    FileUtil back_fu(_back_dir);
    FileUtil pack_fu(_pack_dir);
    back_fu.CreateDirectory(); // CreateDirectory里面会自动判断目录是否已经存在
    pack_fu.CreateDirectory();
  }

  bool RunModule();
private:
  bool HotJudge(const std::string &file);
private:
  int _hot_time; // 热点判断时间差 30s
  std::string _back_dir;
  std::string _pack_dir;
  std::string _pack_suffix;
  std::shared_ptr<ThreadPool> _tp;
};
```

&emsp;

### 2.6 网络通信模块

&emsp;该模块主要负责处理客户端以及浏览器的请求。

**客户端上传请求**: 客户端上传文件至服务器, 服务器读取请求正文内容, 将文件存储到备份目录中。(目录结构文件特殊处理)

**浏览器访问根目录请求**: 返回根目录的文件信息列表。

**浏览器访问子目录请求**: 返回对应子目录**当前层**的文件信息列表。

**浏览器下载请求**: 根据下载URL获取对应文件, 然后将文件内容写入响应正文。(响应时设置响应报头``Accept-Ranges: bytes``告诉客户端服务器支持断点下载功能, 以及响应报头``ETag: xxx``)

```cpp
extern CloudBackup::DataManager *_datam;

class Service{
public:
  Service(){
    Config *cf = Config::GetInstance();
    _server_ip = cf->GetServerIP();
    _server_port = cf->GetServerPort();
    _download_prefix = cf->GetDownloadPrefix();
    _access_prefix = cf->GetAccessPrefix();
  }

  static void Upload(const httplib::Request &req, httplib::Response &rsp);
  static void ShowList(const httplib::Request &req, httplib::Response &rsp);
  static std::string GetETag(const BackupInfo &bi);
  static void Download(const httplib::Request &req, httplib::Response &rsp);
  static void Access(const httplib::Request &req, httplib::Response &rsp);
  bool RunModule();

private:
  std::string _server_ip;
  int _server_port;
  std::string _download_prefix;
  std::string _access_prefix;
  httplib::Server _svr;
};
```

&emsp;

&emsp;

## 3.客户端模块详解

### 3.1 文件实用工具类模块

&emsp;该模块的大部分功能和服务端的文件实用工具类一致。客户端的目的是为了上传文件, 因此客户端不需要对文件进行压缩/解压缩处理, 它的重点在于如何上传文件。客户端额外需要处理一下用户输入的备份路径等信息, 以及为备份目录生成对应的目录结构数据文件。

这里介绍一下该模块客户端**独有的功能**:

1. 处理用户输入的备份目录路径

   > 通过``UnifyDirectoryName()``来获取备份目录的绝对路径, 以方便后续使用。
   >
   > &emsp;``UnifyDirectotyName()``通过调用filesystem的``canonical()``方法获取备份目录的绝对路径, 然后保存到成员变量的``_absolute_path``当中。
   >
   > **问题**: 这里为什么要保存绝对路径呢? 在给服务器上传文件时不是需要相对路径的文件名称吗?
   >
   > **答**: 在``ScanDirectory()``时, 需要扫描备份目录, 此时如果使用相对路径, 那么我们**可能无法得知备份目录的根目录的名称**! 这取决于相对路径的表示。 我们根目录这个目录文件本身也是要在服务器上创建的! 因此这里我们使用绝对路径。
   >
   > &emsp;实际上, ``ScanDirectory()``在设计上使用了``vector<pair<string,string>>``作为参数，这是为了**同时获取绝对路径和相对路径**。**在``Upload``时既需要绝对路径，也需要相对路径**。绝对路径用于打开文件,进行文件的读写。相对路径用于填充``filename``这个字段, 服务端会根据filename创建对应的文件, 因此这里必须是相对路径。
   >
   > &emsp;在``GenerateStructureFile()``时, 我们需要**保存相对路径**到``xxx_dir.stru``文件中，这个文件最终会上传到服务器，服务器会根据文件内容创建多层级目录结构。然而我们只有绝对路径，因此这里需要调用``ConvertAbsolute2Relative()``方法来获取相对路径了!
   >
   > ---
   >
   > **``ConvertAbsolute2Relative()`` 处理路径的过程**:
   >
   > 注意: 该函数处理的不仅仅是目录文件, 普通文件的绝对路径也可以处理! (后面也有使用到)
   >
   > 备份目录: ``E:\\WorkFlow\\DailyNotes\\BackupDir``
   > 处理对象: ``E:\\WorkFlow\\DailyNotes\\BackupDir\\xxx``
   > 处理结果: ``.\\BackupDir\\xxx``
   >
   > ```cpp
   > inline std::string ConvertAbsolute2Relative(const std::string& path) {
   > 	std::string res = ".\\";
   > 	res += path.substr(_absolute_path.size() - FileName().size());
   > 	return res;
   > }
   > ```
   >
   > 

2. 生成目录结构文件

   > &emsp;该文件时针对"当备份目录中存在子目录"的情况所产生的，如果不存在子目录，则目录结构文件内容为空。
   >
   > &emsp;为了支持用户备份的目录下可包含多层级子目录, 因此必须需要一个能够描述备份目录的目录结构信息的数据文件。
   >
   > 该文件生成于备份目录的一级子目录下。名称为``xxx_dir.stru``。
   >
   > 该文件中只保存目录结构信息, 如:``myWork/function\nmyWork/Learning/Kira``。每条目录信息以``\n``为间隔符。目录信息保存的是相对路径(去除前缀``./``), 这里的路径分隔符都是``/``, 做过了处理, 因为最终要上传到服务端, 服务端创建目录时, 识别的目录分隔符是``/``。

```cpp
class FileUtil{
public:
	FileUtil(const std::string &name)
		: _filename(name)
	{
		UnifyDirectoryName();	//保存 目录文件/普通文件 的绝对路径
	}

	const std::string &GetAbsolutePath() { return _absolute_path; }
	const std::string &GetRelativePath() { return _relative_path; }

	bool RemoveFile(const std::string &filename);
	size_t FileSize();
	time_t LastModifyTime();
	time_t LastAccessTime();
	std::string FileName();	//目录文件返回_absolute_path; 普通文件返回_filename
	bool GetPosLenContent(std::string &output_content, size_t pos, size_t len);
	bool GetContent(std::string &output_content);
	bool SetContent(const std::string &content);

	bool Exists();
	bool Create_Directory();	//暂时没有用武之地
	bool ScanDirectory(std::vector<std::pair<std::string, std::string>>& output_array);

private:
    bool GenerateStructureFile();
    inline bool UnifyDirectoryName();
    inline std::string ConvertAbsolute2Relative(const std::string& path);

private:
	std::string _filename;
	struct stat _st;
	std::string _absolute_path;
	//std::string _relative_path;	//目前没有任何用
};
```

&emsp;

### 3.2 数据管理模块

&emsp;与服务端功能一致, 保存已经备份的文件的信息。在内存中依然使用哈希表保存, key为文件的路径(相对路径), value为文件的唯一标识符Tag。Tag的组成为``filename-fsize-LastModifyTime``。每当有新的数据插入/更新时, 会插入到哈希表并同步存储到磁盘的``cloud.dat``文件中。

```cpp
constexpr auto PERSISTENCE_FILE_DATA = "\\cloud.dat";
// 文件唯一标识符Tag: filename-fsize-LastModifyTime
// 客户端只使用一个线程来管理指定目录下的文件，因此这里对于数据管理模块而言，可以不使用锁
class DataManager{
public:
	DataManager(const std::string &backdir)
		: _persistence_file(backdir + PERSISTENCE_FILE_DATA)
	{
		InitLoad();
	}

	bool InitLoad();
	bool Storage();
	bool Insert(const std::string &key, const std::string &val);
	bool Update(const std::string &key, const std::string &val);
	bool GetOneByKey(const std::string &key, std::string &output_val);

private:
	std::vector<std::string> Split(const std::string &s, const std::string &sep);

private:
	std::string _persistence_file = PERSISTENCE_FILE_DATA;	// 备份信息存储文件
	std::unordered_map<std::string, std::string> _path2Tag; // 文件路径?文件唯一标识符Tag
};
```

&emsp;

### 3.3 文件备份模块

&emsp;由于服务器的IP地址和端口号是确定的, 这里直接定义常量``SERVER_IP``和``SERVER_PORT``了。

该模块的主要功能为"检测文件是否需要备份"和"上传文件": 

1. 遍历指定文件夹下的目录
2. 判断文件是否需要备份(新的文件/修改的文件) --- 判断方法(根据文件唯一标识符Tag)
3. 将需要备份的文件上传, 并更改文件的备份信息到(哈希表中)

```cpp
constexpr auto SERVER_IP = "152.136.211.148";
constexpr auto SERVER_PORT = 2333;

class Backup{
public:
	Backup(const std::string &backdir)
		: _back_dir(backdir),
		  _datam(new DataManager(backdir))
	{}
private:
	bool Upload(const std::string& filename);

public:
	bool RunModule(const std::vector<bool>& stops, int index);

private:
	std::string GetFileTag(const std::string &filename);
	bool IsNeedBackup(const std::string &filename);
private:
	std::string _back_dir; // 监控的文件目录(备份文件夹)
	DataManager *_datam;   // 数据管理模块
};
```

&emsp;

### 3.4 目录监控模块

&emsp;监控模块扮演的是"管理者"的角色。它主要是接收来自主线程的信号指令(Add、Delete), 然后执行对应的函数。在Qt界面初始化时，就会创建对应的``Monitor``对象, 因为需要提前加载``MonitoredBackupList.dat``的数据信息。随后, 就等待主线程调用了。

+ ``Insert()``: 将备份目录插入到监控列表并持久化，同时为该备份目录分配对应线程(调用``AddThread()``。
+ ``Delete()``: 将线程对应在stops数组中的位置的flag设置为true，表示该监控目录被删除, 线程会自动销毁。

```cpp
//监控模块的执行时只有主线程操作的. 子线程们是执行备份模块的. 因此监控模块不需要加锁
constexpr auto MONITOR_DAT = "\\MonitoredBackupList.dat";//保存在exe可执行程序的同一级别下
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
	bool InitLoad();
	bool Storage();

public:
	bool Insert(const std::string& path);
	bool Delete(const std::string& path);
	bool Search(const std::string& path);
	std::vector<std::string> ShowMonitorList();

private:
	static void RealTimeBackup(const std::string& path, \
                        const std::vector<bool>& stops, int index);	//线程的执行函数
	int AddThread(const std::string& path);	//返回新添加的线程所在的数组下标 (最后一个位置的下标)

private:
	FileUtil _fu;
	std::vector<std::thread> _ths;
	std::vector<bool> _stops;	//停止标志, 当stop设置为true时, 停止监控该下标所对应的线程
	std::unordered_map<std::string, int> _monitor_list;//备份目录 to 线程所在下标
};
```

&emsp;

### 3.5 Qt交互界面

+ ``Browse``: 浏览本地目录, 用户可以选择要备份的目录 (用户也可以自己手动输入目录, 相对/绝对 路径)
+ ``Add``: 将Backup Path对应的备份目录添加到监控列表中
+ ``Delete``: 将选中的备份目录从监控列表中删除
+ ``ListWidget``: 用于显示备份信息的列表(自带横向、纵向的ScrollBar)

```cpp
class QtCloudBackupClient : public QMainWindow
{
    Q_OBJECT
public:
    QtCloudBackupClient(QWidget *parent = nullptr);
    ~QtCloudBackupClient();
public:
    void browse_on_clicked();
    bool CheckPathValid(const std::string& path);
    bool Backup_Action(bool needCheck);
    void apply_on_clicked();
    void delete_on_clicked();

private:
    Ui::QtCloudBackupClientClass ui;
    CloudBackup::Monitor mon;	//在Qt界面初始化时就加载监控模块对象(因为要读取监控备份列表)
};
```

&emsp;

&emsp;

## 4.待优化问题

1. 我觉得``DataManager``类可以**改成单例类**, 这样就不需要在main函数里面定义一个全局指针对象了。(直接通过``GetInstance()``接口就可以访问到里面的成员)。该类对象全局只需要1个足矣。(**不算什么问题**)
1. 当人为的直接将文件添加到服务器的``backupDir``中时，由于没有更改``cloud.dat``的备份文件信息，因此该文件不会被服务器管理。(无法自动备份、无法在浏览器页面展示) <貌似在``DataManager``模块中，对于``backupDir``存在的文件，而``cloud.dat``中没有记录的文件，它会及时进行更新> (**因此问题可能已经被解决了**)
1. bundle压缩的效率过低，当只存在一个线程执行 热点管理模块中的``RunModule()``方法时，效率太低。它只有在前一个文件压缩了，才会接着压缩后面的。我们可以借助线程池/多线程来提高压缩效率。(**已解决**)
1. 在遍历备份目录时，由于我们实现的``FileUtil::ScanDirectory()``接口是不会递归子目录的。因此当用户想要备份目录下的目录的时候，就无法上传至服务器。我们可以使用递归方法去遍历，但是这样的话，服务器端就需要解决如何根据它的路径来创建相应的目录了。(就是**服务器那边也需要同步目录结构**) --- 但是还需要解决的问题是: 是否需要在前端页面展示出目录文件? 如果要展示目录文件，该如何实现跳转到子目录的功能? (**已实现**)

   > 想法: 
   > **<font color="yellow">Client</font>**:
   >
   > &ensp;完善功能(遗留问题): 
   >
   > 1. 确保客户端输入的备份路径**有效**(在Qt中需要确保客户端输入的路径一定是个目录文件而不是普通文件)。
   > 2. 处理客户端输入的备份路径(**统一化处理**, 将绝对路径和相对路径都变成``./backupDir/xxx``的路径, 其中backupDir是客户端指定的备份目录<什么名字都可能>)
   > 3. 备份信息数据文件``./cloud.dat``文件的生成位置。(改一下路径, 跟随备份目录的位置)
   > 4. 关于大文件拷贝到backupDir的情况，个人认为无需要关心(大不了上传2次)
   > 5. 路径切换
   > 6. 添加备份列表(将客户端提交的所有备份路径存储起来, 动态的监控这些路径下的文件进行自动备份)
   >
   > &emsp;在``ScanDirectory()``时，需要记录一下目录结构(保存到xxx_dir.stru, 这里的xxx是当前目录名称, 这样标识能够确定该文件是否是备份的根目录)。在上传文件时，优先上传xxx_dir.stru。
   >
   > **<font color="yellow">Server</font>**:
   >
   > &emsp;服务端在``Upload()``方法中判断接收的文件，如果是xxx_dir.stru, 则调用``CreateDirectories()``去**递归创建子目录**，直到所有的目录都已经被创建。由于客户端保证了第一个上传的文件一定的是``xxx_dir.stru``，因此我们在处理完目录结构的创建后直接return, 随后接收客户端后续发送的文件。服务端将客户端后续发送的文件依次存放在对应的目录下面。
   >
   > &emsp;服务端**压缩模块**也需要注意一下, 压缩目录可以统一存放到pack_dir，不**过解压的时候需要存放到原本对应的目录下面**。(**在压缩时，目录结构不应该被破坏，即便文件已经被删除了**)
   >
   > &emsp;服务端showlist**页面展示**: 
   >
   > &ensp;完善功能(遗留问题):
   >
   > 1. 路径前缀补充 (**已解决**)
   > 1. 添加日志系统(以便于服务端查看问题和信息)

&emsp;

&emsp;

## 5.项目扩展

1. 考虑使用数据库替代``cloud.dat``持久化存储文件。

2. 考虑当多个用户同时访问服务器时，如何给他们显示不同的页面。即，**如何实现多用户的云备份系统**。(用户管理)

3. 考虑实现让用户同时"监控"同步多个备份目录(真正的实现同步) **√已实现**

   > 实现思路: (额外创建一个Monitor模块)
   >
   > 1. 使用多线程, 当用户每添加一条备份目录时, 就为该备份目录创建一个对应的线程，让该子线程循环检测并备份备份目录下的文件。
   > 2. Qt界面允许用户``Apply``新增要监控的备份目录、``Delete``删除要监控的备份目录。Qt页面需要一个``ListWidget``用来显示当前被监控的备份目录列表。
   > 3. 使用``MonitoredBackupList.dat``存储被监控的备份目录列表。程序每次启动时，需要先加载该列表的信息到内存中(哈希表中), 并为其创建对应的线程以用来监控备份目录。
   > 4. ``MonitoredBackupList.dat``列表应该支持
   >    对外提供使用的方法: ``Insert()``、``Delete()``、``Search()``、``ShowList()``。
   >    内部使用的方法: ``InitLoad()``、``Storage()``、``RealTimeBackup()``、``AddThread()``
   > 5. ``InitLoad()``和``Insert()``都会涉及到插入备份目录信息, 此时也会调用``AddThread()``去监控该备份目录。
   > 6. ``Delete()``会将``stop``标志位设置为true, 此时在循环中的线程就能够从循环中跳出来了。(线程已经是detach的了)

---

- [x] 给客户端开发一个好看的界面，让监控目录可以选择(用户自己输入路径) --- **Qt实现**
- [ ] 内存中的管理的数据也可以采用热点管理
- [x] 压缩模块可以使用线程池实现 (仅压缩; 解压模块不推荐使用线程池,容易出问题)
- [x] 允许上传多层级目录结构下的文件
- [ ] 实现用户管理，不同的用户分文件夹存储以及查看
