#include <android/log.h>
//#include <stdlib.h>
#include <jni.h>
#include <dlfcn.h>
#include "../include/dlopen.h"
#include "../include/mhk.h"
//#include "../include/iModel.h"
#include "../arm64/model/iModel.h"


#define LOG_TAG "lybin"

#define LE(fmt, args...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, fmt, ##args);

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
//    LE("tid=%d onPreCallBack:%s", gettid(), name);

#if defined(__aarch64__)

    LE("tid=%d, onPreCallBack:%s, "
       "x0=0x%llx, x1=0x%llx, x2=0x%llx, x3=0x%llx, x4=0x%llx, x5=0x%llx, x6=0x%llx, x7=0x%llx, x8=0x%llx, x9=0x%llx, x10=0x%llx, "
       "x11=0x%llx, x12=0x%llx, x13=0x%llx, x14=0x%llx, x15=0x%llx, x16=0x%llx, x17=0x%llx, x18=0x%llx, x19=0x%llx, x20=0x%llx, "
       "x21=0x%llx, x22=0x%llx, x23=0x%llx, x24=0x%llx, x25=0x%llx, x26=0x%llx, x27=0x%llx, x28=0x%llx, x29/FP=0x%llx, x30/LR=0x%llx, "
       "cur_sp=%p, ori_sp=%p, ori_sp/31=0x%llx, NZCV/32=0x%llx, x0/pc/33=0x%llx, cur_pc=%llx, arg8=%x, arg9=%x, arg10=%x, arg11=%x, "
       "arg12=%x, arg13=%x;"
    , gettid(), name,
       regs->uregs[0], regs->uregs[1], regs->uregs[2], regs->uregs[3], regs->uregs[4], regs->uregs[5],
       regs->uregs[6], regs->uregs[7], regs->uregs[8], regs->uregs[9], regs->uregs[10], regs->uregs[11],
       regs->uregs[12], regs->uregs[13], regs->uregs[14], regs->uregs[15], regs->uregs[16], regs->uregs[17],
       regs->uregs[18], regs->uregs[19], regs->uregs[20], regs->uregs[21], regs->uregs[22], regs->uregs[23],
       regs->uregs[24], regs->uregs[25], regs->uregs[26], regs->uregs[27], regs->uregs[28], regs->uregs[29], regs->uregs[30],
       regs, /*((char*)regs + 0x110)*/((char*)regs + 0x310), regs->uregs[31], regs->uregs[32], regs->uregs[33], regs->pc,
       SP(0), SP(1), SP(2), SP(3), SP(4), SP(5)
    );
    uint8_t *base = reinterpret_cast<uint8_t *>(regs->uregs[0]);
    size_t size = regs->uregs[1];
    LE("base,X0=%s,x0=%x", regs->uregs[0], regs->uregs[0]);
    LE("size,X1=%d", regs->uregs[1]);
    LE("X2=%s", regs->uregs[2]);
    LE("X3=%d", regs->uregs[3]);
//    if (size < 102400L) {
//        LE( "new_arm64_open--size<DEX_MIN_LEN");
//    } else {
//        LE( "new_arm64_open--save_dex_file");
    save_dex_file(base, size);
//    }
#endif
}

extern "C" JNIEXPORT void JNICALL
Java_com_lybin_android_xposed_shelling_Native_test(JNIEnv *env, jclass type,jstring pn) {
    char *p = (char *) env->GetStringUTFChars(pn, 0);
    LE("native called:%s",p);
    //   保存包名
    init_package_name(p);
//    释放p占用的java内存
    env->ReleaseStringChars(pn, (const jchar *) p);
    ndk_init(env);

    void *handle = ndk_dlopen("libart.so", RTLD_NOW);
    char *open_common = "_ZN3art7DexFile10OpenCommonEPKhmRKNSt3__112basic_stringIcNS3_11char_traitsIcEENS3_9allocatorIcEEEEjPKNS_10OatDexFileEbbPS9_PNS0_12VerifyResultE";
    char *open ="_ZN3art7DexFile4OpenEPKhmRKNSt3__112basic_stringIcNS3_11char_traitsIcEENS3_9allocatorIcEEEEjPKNS_10OatDexFileEbbPS9_";
    void *open_addr = ndk_dlsym(handle, open_common);
    LE("ndk_dlopen--libart.so--handle=%p", handle);
    LE("ndk_dlsym--open_common_addr=%p", open_addr);
    dump((void*)(open_addr),openMethodCallBack, NULL,"openCommon");
}


