LOCAL_PATH := $(call my-dir)

$(call emugl-begin-shared-library,libOpenglSystemCommon)
$(call emugl-import,libGLESv1_enc libGLESv2_enc lib_renderControl_enc)
ifeq (true,$(GOLDFISH_OPENGL_BUILD_FOR_HOST))
$(call emugl-import,libplatform$(GOLDFISH_OPENGL_LIB_SUFFIX))
$(call emugl-import,libGoldfishAddressSpace$(GOLDFISH_OPENGL_LIB_SUFFIX))
$(call emugl-import,libqemupipe$(GOLDFISH_OPENGL_LIB_SUFFIX))
$(call emugl-import,libgralloc_cb$(GOLDFISH_OPENGL_LIB_SUFFIX))
else
$(call emugl-export,STATIC_LIBRARIES,libplatform)
$(call emugl-export,STATIC_LIBRARIES,libGoldfishAddressSpace libringbuffer)
$(call emugl-export,STATIC_LIBRARIES,libqemupipe.ranchu)
$(call emugl-export,HEADER_LIBRARIES,libgralloc_cb.ranchu)
endif

LOCAL_SRC_FILES := \
    FormatConversions.cpp \
    HostConnection.cpp \
    QemuPipeStream.cpp \
    ProcessPipe.cpp    \
    ThreadInfo.cpp \

ifeq (true,$(GFXSTREAM))
$(call emugl-import,libvulkan_enc)

LOCAL_SRC_FILES += AddressSpaceStream.cpp

endif

LOCAL_CFLAGS += -Wno-unused-variable -Wno-unused-parameter -fno-emulated-tls

ifeq (true,$(GOLDFISH_OPENGL_BUILD_FOR_HOST))

else

ifeq (true,$(GFXSTREAM))

LOCAL_HEADER_LIBRARIES += gfxstream_vulkan_headers

LOCAL_CFLAGS += -DVIRTIO_GPU
LOCAL_SRC_FILES += \
    VirtioGpuPipeStream.cpp \

LOCAL_C_INCLUDES += external/libdrm external/minigbm/cros_gralloc
LOCAL_SHARED_LIBRARIES += libdrm

endif

endif

ifdef IS_AT_LEAST_OPD1
LOCAL_HEADER_LIBRARIES += libnativebase_headers

$(call emugl-export,HEADER_LIBRARIES,libnativebase_headers)
endif

ifdef IS_AT_LEAST_OPD1
LOCAL_HEADER_LIBRARIES += libhardware_headers
$(call emugl-export,HEADER_LIBRARIES,libhardware_headers)
endif

$(call emugl-export,C_INCLUDES,$(LOCAL_PATH)/bionic-include)
$(call emugl-export,C_INCLUDES,$(LOCAL_PATH) bionic/libc/private)
$(call emugl-export,C_INCLUDES,$(LOCAL_PATH) bionic/libc/platform)

ifeq (true,$(GOLDFISH_OPENGL_BUILD_FOR_HOST))
$(call emugl-export,SHARED_LIBRARIES,android-emu-shared)
endif

$(call emugl-end-module)
