/*
 *
 *  @author : rrrfff@foxmail.com
 *  https://github.com/rrrfff/ndk_dlopen
 *
 */
#pragma once

#include <jni.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <android/log.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#ifdef __cplusplus
extern "C" {
#endif

void ndk_init(JNIEnv *env);
void *ndk_dlopen(const char *filename, int flag);
int ndk_dlclose(void *handle);
const char *ndk_dlerror(void);
void *ndk_dlsym(void *handle, const char *symbol);
int ndk_dladdr(const void *ddr, Dl_info *info);
void save_dex_file( u_int8_t *data, size_t length);
void init_package_name(char *package_name);
void test_log(char* str);
#ifdef __cplusplus
}
#endif