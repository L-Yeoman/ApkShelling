## ApkShelling 脱壳原理
该工具目前仅支持安卓5.0、6.0 版本脱壳使用：


1.hook ClassLoader的loadClass方法


2.反射获取Class的getDex(),获取dex


3.反射获取Dex的getBytes(),保存dex文件



修复脱壳后dex文件，修改dex头文件Magic Value（dex文件头、035文件格式版本）

---
## 8.0后Dump流程
支持安卓8.0及以上版本脱壳使用，通过edxposed进程注入,hook libart.so中openCommon()，不同版本签名方法不同，需要用ida查看，使用时替换hook方法名。

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
## inlineHook 基本实现
读写寄存器、堆栈。接收读写寄存器和堆栈，实现dump的目的，原函数继续运行，在不影响原函数运行情况下实现hook，避免无法多次调用同一个被hook函数的调用异常。

- 备份原函数代码段
```CPP
    //arm模式下为24
    pInfo->backUpLength = 24;
    memcpy(pInfo->szbyBackupOpcodes, pInfo->pBeHookAddr, pInfo->backUpLength);
```

- malloc shellcode汇编代码到内存段
```CPP
         //malloc一段新的stub代码
        long pagesize = sysconf(_SC_PAGE_SIZE);
        void *pNewShellCode = NULL;// = malloc(sShellCodeLength);
        int code = posix_memalign(&pNewShellCode, pagesize, pagesize);

        LE("pNewShellCode=%p", pNewShellCode);
        if(code || pNewShellCode == NULL)
        {
            LOGI("shell code malloc fail.");
            break;
        }
        memcpy(pNewShellCode, p_shellcode_start_s, sShellCodeLength);
```
- 填充shellcode执行之后的回调地址
```CPP
   BYTE szLdrPCOpcodes[24] = {0xe1, 0x03, 0x3f, 0xa9, 0x40, 0x00, 0x00, 0x58, 0x00, 0x00, 0x1f, 0xd6};
        //将目的地址拷贝到跳转指令缓存位置
        memcpy(szLdrPCOpcodes + 12, &pJumpAddress, 8);
        szLdrPCOpcodes[20] = 0xE0;
        szLdrPCOpcodes[21] = 0x83;
        szLdrPCOpcodes[22] = 0x5F;
        szLdrPCOpcodes[23] = 0xF8;
        LOGI("LIVE4.3.4");
        
        //将构造好的跳转指令刷进去
        memcpy(pCurAddress, szLdrPCOpcodes, 24);
```
- 最后构建跳转，在方法被调用时，跳转到shellcode代码段
```CPP
        //修改原位置的页属性，保证可写
        if(ChangePageProperty(pInfo->pBeHookAddr, 24) == false)
        {
            LOGI("change page property error.");
            break;
        }
        LOGI("LIVE4.3");
        //填充跳转指令
        if(BuildArmJumpCode(pInfo->pBeHookAddr, pInfo->pStubShellCodeAddr, pInfo) == false)
```
- hook回调，param1-param7从x0-x6取，更多参数从栈中取
```CPP
void openMethodCallBack(my_pt_regs *regs, HK_INFO *pInfo) //参数regs就是指向栈上的一个数据结构，由第二部分的mov r0, sp所传递。
{
    const char *name = "null";
    if (pInfo) {
        if (pInfo->methodName) {
            name = pInfo->methodName;
        } else {
            char buf[20];
            sprintf(buf, "%p", pInfo->pBeHookAddr);
            name = buf;
        }
    }
    LE("tid=%d, onPreCallBack:%s, x0=0x%llx, x1=0x%llx",gettid(), name,
       regs->uregs[0], regs->uregs[1]);
```
- shellcode核心段，拉伸栈空间，保存真实sp、状态寄存器、LR。再次移动栈帧，将x0-x29寄存器按照低地址向高地址的方向从下向上依次存入，回调hook方法传入sp和结构指针作为参数。恢复寄存器，恢复栈平衡，跳转原函数继续执行。
```CPP
_dump_start:                    //用于读写寄存器/栈，需要自己解析参数，不能读写返回值，不能阻止原函数(被hook函数)的执行
                                //从行为上来我觉得更偏向dump，所以起名为dump。
    sub     sp, sp, #0x20;      //sp=sp-0x20,拉伸栈空间，跳板在栈上存储了x0、x1，但是未改变sp的值

    mrs     x0, NZCV            //将系统的NZCV标志位状态寄存器移植到通用寄存器
    str     x0, [sp, #0x10];    //覆盖跳板存储的x1，存储状态寄存器
    str     x30, [sp];          //存储x30
    add     x30, sp, #0x20      //sp+0x20,sp初始地址
    str     x30, [sp, #0x8];    //存储真实的sp
    ldr     x0, [sp, #0x18];    //取出跳板存储的x0
save_x0_x29://保存寄存器x0-x29
    sub     sp, sp, #0xf0;      //分配栈空间,向下递减
    stp     X0, X1, [SP];       //存储x0-x29
    stp     X2, X3, [SP,#0x10]
    ...
call_onPreCallBack://调用onPreCallBack函数，第一个参数是sp，第二个参数是STR_HK_INFO结构体指针
    mov     x0, sp;                 //x0作为第一个参数，那么操作x0=sp，即操作栈读写保存的寄存器
    ldr     x1, _hk_info;
    ldr     x3, [x1];               //onPreCallBack
    bl      get_lr_pc;              //返回地址保存到lr，lr为下条指令
    add     lr, lr, #8;              //lr为blr x3的地址
    str     lr, [sp, #0x108];        //lr当作pc，覆盖栈上的x0
    blr     x3                       //绝对跳转X3，返回地址保存到LR
restore_regs://恢复寄存器
    ldr     x0, [sp, #0x100];       //取出状态寄存器
    msr     NZCV, x0
    ...
```
## 使用方法
- 将libshelling.so复制到/data/local/tmp目录下
- chmod 777 libshelling.so  赋值读写执行权限
- setenforce 0  修改linux防火墙
- 在XposedManager中选中模块
- 脱壳成功，dex路径/data/data/{packagename}/dump
---
## 相关文章
[如何写一个Android inline hook框架](https://bbs.pediy.com/thread-257020.htm)

[SandHook](https://github.com/ganyao114/SandHook)

[ARMv8 架构与指令集](https://blog.csdn.net/forever_2015/article/details/50285865)

