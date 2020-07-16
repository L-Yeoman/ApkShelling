LOCAL_PATH := $(call my-dir)  #此变量表示源文件在开发树中的位置

include $(CLEAR_VARS)

$(warning "abi: $(TARGET_ARCH_ABI)")

#ifeq "$(TARGET_ARCH_ABI)" "arm64-v8a"

#LOCAL_CXXFLAGS +=  -g -O0
#LOCAL_ARM_MODE := arm
LOCAL_MODULE    		:= shelling  #so 名称
LOCAL_STATIC_LIBRARIES	:= imodel #当前模块依赖的静态库模块列表 dump_with_ret dump replace
LOCAL_C_INCLUDES 		:= $(LOCAL_PATH)/../include #使用此可选变量指定相对于 NDK root 目录的路径列表
LOCAL_SRC_FILES 		:= main.cpp	dlopen.c

LOCAL_LDLIBS 			+= -L$(SYSROOT)/usr/lib -llog #指定共享库链接器

include $(BUILD_SHARED_LIBRARY)

#endif