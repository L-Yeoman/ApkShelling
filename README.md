# ApkShelling 脱壳原理
该工具目前仅支持安卓5.0、6.0 版本脱壳使用：


1.hook ClassLoader的loadClass方法


2.反射获取Class的getDex(),获取dex


3.反射获取Dex的getBytes(),保存dex文件



修复脱壳后dex文件，修改dex头文件Magic Value（dex文件头、035文件格式版本）

---
# 8.0后Dump流程
支持安卓8.0及以上版本脱壳使用，hook libart.so中openCommon()，不同版本签名方法不同，需要用ida查看，使用时替换hook方法名。

- 跳过7.0后的私有api限制，获取libart.so内存段的加载位置
  ```CPP
    static __attribute__((__aligned__(PAGE_SIZE))) uint8_t __insns[PAGE_SIZE];
    STUBS.generic_stub = __insns;
    //修改内存区读写属性
    mprotect(__insns, sizeof(__insns), PROT_READ | PROT_WRITE | PROT_EXEC);
    /*对dlopen在内存段上替换，打到hook目底，获取so加载真实位置
      destin-- 指向用于存储复制内容的目标数组，类型强制转换为 void* 指针。
      source-- 指向要复制的数据源，类型强制转换为 void* 指针。
      n-- 要被复制的字节数。*/
    memcpy(__insns, "\xFD\x7B\xBF\xA9\xFD\x03\x00\x91\xFE\x03\x02\xAA\x60\x00\x1F\xD6", 16);
    memcpy(quick_on_stack_back, "\xFD\x7B\xC1\xA8\xC0\x03\x5F\xD6", 8);
  ```
- 通过动态符号表，查询hook方法的内存偏移
 ```CPP
    #if defined(__i386__) || defined(__x86_64__) || defined(__aarch64__) || defined(__arm__)
        return STUBS.quick_on_stack_replace(handle, symbol,
                                            quick_on_stack_back, dlsym);
 ```
- hook签名方法的内存地址
```CPP
    //HOOk地址不可空
   dump((void*)(open_addr),openMethodCallBack, NULL,"openCommon");
```
---
# inlineHook 基本实现

# 使用方法

---
# 相关文章
[如何写一个Android inline hook框架](https://bbs.pediy.com/thread-257020.htm)
[SandHook](https://github.com/ganyao114/SandHook)
