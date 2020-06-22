# ApkShelling 脱壳原理
该工具目前仅支持安卓5.0、6.0 版本脱壳使用：


1.hook ClassLoader的loadClass方法


2.反射获取Class的getDex(),获取dex


3.反射获取Dex的getBytes(),保存dex文件



修复脱壳后dex文件，修改dex头文件Magic Value（dex文件头、035文件格式版本）