

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