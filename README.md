[toc]

# 文件云同步系统



## bundle库实现文件解压缩

[r-lyeh-archived bundle库](https://github.com/r-lyeh-archived/bundle)

**<font size="4">• API接口</font>**

```cpp
namespace bundle
{
  // low level API (raw pointers)
  bool is_packed( *ptr, len );
  bool is_unpacked( *ptr, len );
  unsigned type_of( *ptr, len );
  size_t len( *ptr, len );
  size_t zlen( *ptr, len );
  const void *zptr( *ptr, len );
  bool pack( unsigned Q, *in, len, *out, &zlen );
  bool unpack( unsigned Q, *in, len, *out, &zlen );

  // medium level API, templates (in-place)
  bool is_packed( T );
  bool is_unpacked( T );
  unsigned type_of( T );
  size_t len( T );
  size_t zlen( T );
  const void *zptr( T );
  bool unpack( T &, T );
  bool pack( unsigned Q, T &, T );

  // high level API, templates (copy)
  T pack( unsigned Q, T );
  T unpack( T );
}
```

&emsp;我们这里使用high level API，``pack``与``unpack``。

&emsp;

&emsp;

## 服务端配置信息模块







## 待优化问题

1. 我觉得``DataManager``类可以**改成单例类**, 这样就不需要在main函数里面定义一个全局指针对象了。(直接通过``GetInstance()``接口就可以访问到里面的成员)。该类对象全局只需要1个足矣。
1. 当人为的直接将文件添加到服务器的``backupDir``中时，由于没有更改``cloud.dat``的备份文件信息，因此该文件不会被服务器管理。(无法自动备份、无法在浏览器页面展示) <貌似在``DataManager``模块中，对于``backupDir``存在的文件，而``cloud.dat``中没有记录的文件，它会及时进行更新> (**因此问题可能已经被解决了**)
1. bundle压缩的效率过低，当只存在一个线程执行 热点管理模块中的``RunModule()``方法时，效率太低。它只有在前一个文件压缩了，才会接着压缩后面的。我们可以借助线程池/多线程来提高压缩效率。
1. 在遍历备份目录时，由于我们实现的``FileUtil::ScanDirectory()``接口是不会递归子目录的。因此当用户想要备份目录下的目录的时候，就无法上传至服务器。我们可以使用递归方法去遍历，但是这样的话，服务器端就需要解决如何根据它的路径来创建相应的目录了。(就是**服务器那边也需要同步目录结构**) --- 但是还需要解决的问题是: 是否需要在前端页面展示出目录文件? 如果要展示目录文件，该如何实现跳转到子目录的功能?

&emsp;

&emsp;

## 项目扩展

1. 考虑使用数据库替代``cloud.dat``持久化存储文件。
2. 考虑当多个用户同时访问服务器时，如何给他们显示不同的页面。即，**如何实现多用户的云备份系统**。(用户管理)

---

- [x] 给客户端开发一个好看的界面，让监控目录可以选择(用户自己输入路径) --- **Qt实现**
- [ ] 内存中的管理的数据也可以采用热点管理
- [ ] 压缩模块也可以使用线程池实现
- [ ] 实现用户管理，不同的用户分文件夹存储以及查看
- [ ] 实现断点上传
- [ ] 客户端限速，收费则放开
