
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

TARGET_PLATFORM  := $(APP_PLATFORM)
LOCAL_MODULE     := gls4
LOCAL_ARM_MODE   := thumb
#LOCAL_CPPFLAGS   := -DPLATFORM_ANDROID -Wno-invalid-offsetof
LOCAL_CPPFLAGS   := -DPLATFORM_ANDROID -DANDROID_NDK_VER=$(ANDROID_NDK_VER) -Wno-invalid-offsetof -fexceptions -frtti

ifeq ($(strip $(APP_PLATFORM)),android-29)
	LOCAL_CPPFLAGS += -DANDROID_API_LEVEL=29
else
	ifeq ($(strip $(APP_PLATFORM)),android-21)
		LOCAL_CPPFLAGS += -DANDROID_API_LEVEL=21
	else
		ifeq ($(strip $(APP_PLATFORM)),android-19)
			LOCAL_CPPFLAGS += -DANDROID_API_LEVEL=19
		else
			LOCAL_CPPFLAGS += -DANDROID_API_LEVEL=8
		endif
	endif
endif

ifeq ($(strip $(TARGET_ARCH_ABI)),arm64-v8a)
	LOCAL_CPPFLAGS += -D__arm__=1 -D__aarch64__=1
endif
ifeq ($(strip $(TARGET_ARCH_ABI)),armeabi-v7a)
	LOCAL_CPPFLAGS += -D__arm__=1 -D__TARGET_ARCH_ARM=7 -D__TARGET_ARCH_THUMB=4
endif
ifeq ($(strip $(TARGET_ARCH_ABI)),x86)
	LOCAL_CPPFLAGS += -D__i386__=1
endif
ifeq ($(strip $(TARGET_ARCH_ABI)),x86_64)
	LOCAL_CPPFLAGS += -D__x86_64__=1
endif
ifeq ($(strip $(TARGET_ARCH_ABI)),mips64)
	LOCAL_CPPFLAGS += -D__mips64__=1
endif

LOCAL_C_INCLUDES := $(ANDROID_NDK_HOME)/sources/android/cpufeatures
LOCAL_C_INCLUDES += $(TINYGLTF_HOME)
LOCAL_C_INCLUDES += $(COTOPHA_HOME)/Include/common
LOCAL_C_INCLUDES += $(COTOPHA_HOME)/Include/unix
LOCAL_C_INCLUDES += $(COTOPHA_HOME)/Include/opengl
LOCAL_C_INCLUDES += $(COTOPHA_HOME)/Include/android
LOCAL_C_INCLUDES += $(COTOPHA_HOME)/Include/android/gls4jclass


FILE_LIST := $(wildcard $(COTOPHA_HOME)/Source/common/esl/*.cpp)
FILE_LIST += $(wildcard $(COTOPHA_HOME)/Source/android/esl/*.cpp)
FILE_LIST += $(wildcard $(COTOPHA_HOME)/Source/common/sakura/*.cpp)
FILE_LIST += $(wildcard $(COTOPHA_HOME)/Source/android/sakura/*.cpp)
#FILE_LIST += $(wildcard $(COTOPHA_HOME)/Source/android/gls4jclass/*.cpp)
FILE_LIST += $(wildcard $(COTOPHA_HOME)/Source/common/sakuracl/erisa/*.cpp)
FILE_LIST += $(wildcard $(COTOPHA_HOME)/Source/common/sakuragl/*.cpp)
FILE_LIST += $(wildcard $(COTOPHA_HOME)/Source/opengl/sakuragl/*.cpp)
FILE_LIST += $(wildcard $(COTOPHA_HOME)/Source/android/sakuragl/*.cpp)
FILE_LIST += $(wildcard $(COTOPHA_HOME)/Source/common/sakuragl/erisa/*.cpp)
FILE_LIST += $(wildcard $(COTOPHA_HOME)/Source/common/sakuragl/media/*.cpp)
FILE_LIST += $(wildcard $(COTOPHA_HOME)/Source/common/sakuragl/sgl2d/*.cpp)
FILE_LIST += $(wildcard $(COTOPHA_HOME)/Source/common/sakuragl/sgl3d/*.cpp)
FILE_LIST += $(wildcard $(COTOPHA_HOME)/Source/common/sakuragl/window/*.cpp)
FILE_LIST += $(wildcard $(COTOPHA_HOME)/Source/common/sakuraglx/*.cpp)
FILE_LIST += $(wildcard $(COTOPHA_HOME)/Source/common/sakuraglx/ui/*.cpp)
FILE_LIST += $(wildcard $(COTOPHA_HOME)/Source/common/sakuraglx/sprite/*.cpp)
FILE_LIST += $(wildcard $(COTOPHA_HOME)/Source/common/sakuraglx/render/*.cpp)
FILE_LIST += $(wildcard $(COTOPHA_HOME)/Source/common/sakuraglx/extra/*.cpp)
FILE_LIST += $(wildcard $(COTOPHA_HOME)/Source/common/glscs/*.cpp)
FILE_LIST += $(wildcard $(COTOPHA_HOME)/Source/common/rosetta/*.cpp)
FILE_LIST += $(wildcard $(COTOPHA_HOME)/Source/common/loquaty/*.cpp)
FILE_LIST += $(wildcard $(COTOPHA_HOME)/Source/common/antirrhinum/*.cpp)
LOCAL_SRC_FILES += $(FILE_LIST:$(LOCAL_PATH)/%=%)


ifeq ($(strip $(TARGET_ARCH_ABI)),armeabi-v7a)
	FILE_LIST := $(COTOPHA_HOME)/Source/common/sakuragl/sgl3d/neon/sgl3d_matrix_neon.cpp.neon
	FILE_LIST += $(COTOPHA_HOME)/Source/common/sakuraglx/render/neon/sglx3d_collision_neon.cpp.neon
	FILE_LIST += $(COTOPHA_HOME)/Source/common/sakuracl/erisa/scl_erisa_matrix_armv7a.s
	FILE_LIST += $(COTOPHA_HOME)/Source/common/sakuracl/erisa/scl_erisa_matrix_arm_neon.s.neon
	LOCAL_SRC_FILES += $(FILE_LIST:$(LOCAL_PATH)/%=%)
endif

ifeq ($(strip $(TARGET_ARCH_ABI)),arm64-v8a)
	FILE_LIST := $(COTOPHA_HOME)/Source/common/sakuragl/sgl3d/neon/sgl3d_matrix_neon.cpp.neon
	FILE_LIST += $(COTOPHA_HOME)/Source/common/sakuraglx/render/neon/sglx3d_collision_neon.cpp.neon
	LOCAL_SRC_FILES += $(FILE_LIST:$(LOCAL_PATH)/%=%)
endif



# add loquaty library
LOCAL_C_INCLUDES += $(LOQUATY_HOME)/Loquaty/include

FILE_LIST := $(wildcard $(LOQUATY_HOME)/Loquaty/source/*.cpp)
#LOCAL_SRC_FILES += $(FILE_LIST:$(LOCAL_PATH)/%=%)



#LOCAL_LDLIBS    := -llog -lGLESv1_CM
ifeq ($(strip $(APP_PLATFORM)),android-9)
#	LOCAL_LDLIBS +=  -lGLESv2
endif
ifeq ($(strip $(APP_PLATFORM)),android-18)
#	LOCAL_LDLIBS += -lGLESv3
endif


include $(BUILD_STATIC_LIBRARY)


