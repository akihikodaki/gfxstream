#!/usr/bin/python3 -i
#
# Copyright (c) 2013-2018 The Khronos Group Inc.
# Copyright (c) 2013-2018 Google Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import os, re, sys
from generator import *

import cereal

# CerealGenerator - generates set of driver sources
# while being agnostic to the stream implementation

copyrightHeader = """// Copyright (C) 2018 The Android Open Source Project
// Copyright (C) 2018 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
"""

autogeneratedHeaderTemplate = """
// Autogenerated module %s
// %s
// Please do not modify directly;
// re-run android/scripts/generate-vulkan-sources.sh,
// or directly from Python by defining:
// VULKAN_REGISTRY_XML_DIR : Directory containing genvk.py and vk.xml
// CEREAL_OUTPUT_DIR: Where to put the generated sources.
// python3 $VULKAN_REGISTRY_XML_DIR/genvk.py -registry $VULKAN_REGISTRY_XML_DIR/vk.xml cereal -o $CEREAL_OUTPUT_DIR
"""

autogeneratedMkTemplate = """
# Autogenerated makefile
# %s
# Please do not modify directly;
# re-run android/scripts/generate-vulkan-sources.sh,
# or directly from Python by defining:
# VULKAN_REGISTRY_XML_DIR : Directory containing genvk.py and vk.xml
# CEREAL_OUTPUT_DIR: Where to put the generated sources.
# python3 $VULKAN_REGISTRY_XML_DIR/genvk.py -registry $VULKAN_REGISTRY_XML_DIR/vk.xml cereal -o $CEREAL_OUTPUT_DIR
"""

def banner_command(argv):
    """Return sanitized command-line description.
       |argv| must be a list of command-line parameters, e.g. sys.argv.
       Return a string corresponding to the command, with platform-specific
       paths removed."""

    def makeRelative(someArg):
        if os.path.exists(someArg):
            return os.path.relpath(someArg)
        return someArg

    return ' '.join(map(makeRelative, argv))

# ---- methods overriding base class ----
# beginFile(genOpts)
# endFile()
# beginFeature(interface, emit)
# endFeature()
# genType(typeinfo,name)
# genStruct(typeinfo,name)
# genGroup(groupinfo,name)
# genEnum(enuminfo, name)
# genCmd(cmdinfo)
class CerealGenerator(OutputGenerator):

    """Generate serialization code"""
    def __init__(self, errFile = sys.stderr,
                       warnFile = sys.stderr,
                       diagFile = sys.stdout):
        OutputGenerator.__init__(self, errFile, warnFile, diagFile)

        self.typeInfo = cereal.VulkanTypeInfo()

        self.modules = {}
        self.moduleList = []

        self.wrappers = []

        self.codegen = cereal.CodeGen()

        self.host_cmake_generator = lambda cppFiles: """%s
set(OpenglRender_vulkan_cereal_src %s)
android_add_library(OpenglRender_vulkan_cereal)
target_link_libraries(OpenglRender_vulkan_cereal PRIVATE emugl_base)
target_link_libraries(OpenglRender_vulkan_cereal PUBLIC android-emu-base)

target_include_directories(OpenglRender_vulkan_cereal
                           PUBLIC
                           ${ANDROID_EMUGL_DIR}/host/libs/libOpenglRender/vulkan/cereal
                           PRIVATE
                           ${ANDROID_EMUGL_DIR}/host/include
                           ${ANDROID_EMUGL_DIR}/host/libs/libOpenglRender/
                           ${ANDROID_EMUGL_DIR}/host/include/vulkan)
""" % (autogeneratedMkTemplate % banner_command(sys.argv), cppFiles)

        self.guest_android_mk_generator = lambda cppFiles: """%s
LOCAL_PATH := $(call my-dir)

$(call emugl-begin-shared-library,libvulkan_enc)
$(call emugl-export,C_INCLUDES,$(LOCAL_PATH))
$(call emugl-import,libOpenglCodecCommon$(GOLDFISH_OPENGL_LIB_SUFFIX) libandroidemu)

# Vulkan include dir
ifeq (true,$(GOLDFISH_OPENGL_BUILD_FOR_HOST))
LOCAL_C_INCLUDES += \\
    $(LOCAL_PATH) \\
    $(HOST_EMUGL_PATH)/host/include \\
    $(HOST_EMUGL_PATH)/host/include/vulkan
endif

ifneq (true,$(GOLDFISH_OPENGL_BUILD_FOR_HOST))
LOCAL_C_INCLUDES += \\
    $(LOCAL_PATH) \\
    $(LOCAL_PATH)/../vulkan_enc \\

LOCAL_HEADER_LIBRARIES += \\
    vulkan_headers \\

endif

LOCAL_CFLAGS += \\
    -DLOG_TAG=\\"goldfish_vulkan\\" \\
    -Wno-missing-field-initializers \\
    -fstrict-aliasing \\
    -DVK_USE_PLATFORM_ANDROID_KHR \\
    -DVK_NO_PROTOTYPES \\

LOCAL_SRC_FILES := VulkanStream.cpp \\
    %s

$(call emugl-end-module)
"""% (autogeneratedMkTemplate % banner_command(sys.argv), cppFiles)

        encoderInclude = """
#include <memory>
class IOStream;
"""
        encoderImplInclude = """
#include "IOStream.h"
#include "VulkanStream.h"

#include "android/base/AlignedBuf.h"

#include "goldfish_vk_marshaling_guest.h"
"""
        marshalIncludeGuest = """
#include "goldfish_vk_marshaling_guest.h"
#include "VulkanStream.h"

// Stuff we are not going to use but if included,
// will cause compile errors. These are Android Vulkan
// required extensions, but the approach will be to
// implement them completely on the guest side.
#undef VK_KHR_android_surface
#undef VK_ANDROID_external_memory_android_hardware_buffer
"""
        vulkanStreamInclude = """
#include "VulkanStream.h"
#include "android/base/files/StreamSerializing.h"
"""
        testingInclude = """
#include <string.h>
#include <functional>
using OnFailCompareFunc = std::function<void(const char*)>;
"""
        poolInclude = """
#include "android/base/Pool.h"
using android::base::Pool;
"""
        handleMapInclude = """
#include "VulkanHandleMapping.h"
"""
        dispatchHeaderDefs = """
namespace goldfish_vk {

struct VulkanDispatch;

} // namespace goldfish_vk
using DlOpenFunc = void* (void);
using DlSymFunc = void* (void*, const char*);
"""

        dispatchImplIncludes = """
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
"""

        decoderHeaderIncludes = """
#include <memory>
"""

        decoderImplIncludes = """
#include "common/goldfish_vk_marshaling.h"

#include "IOStream.h"
#include "emugl/common/logging.h"

#include "VulkanDispatch.h"
#include "VulkanStream.h"

"""

        self.guest_tag = "guest"
        self.host_tag = "host"

        self.guest_abs_destination = \
            os.path.join(
                os.getcwd(),
                "..", "..",
                "device", "generic", "goldfish-opengl",
                "system", "vulkan_enc")
        self.host_abs_decoder_destination = \
            os.path.join(
                os.getcwd(),
                "android", "android-emugl", "host",
                "libs", "libOpenglRender")

        self.addGuestModule(
            "VkEncoder",
            extraHeader = encoderInclude,
            extraImpl = encoderImplInclude,
            useNamespace = False)
        self.addGuestModule("goldfish_vk_marshaling_guest",
                            extraHeader=marshalIncludeGuest)
        self.addModule("common", "goldfish_vk_marshaling",
                       extraHeader=vulkanStreamInclude)
        self.addModule("common", "goldfish_vk_testing",
                       extraHeader=testingInclude)
        self.addModule("common", "goldfish_vk_deepcopy",
                       extraHeader=poolInclude)
        self.addModule("common", "goldfish_vk_handlemap",
                       extraHeader=handleMapInclude)
        self.addModule("common", "goldfish_vk_dispatch",
                       extraHeader=dispatchHeaderDefs,
                       extraImpl=dispatchImplIncludes)
        self.addHostModule("VkDecoder",
                           extraHeader=decoderHeaderIncludes,
                           extraImpl=decoderImplIncludes,
                           useNamespace=False)

        self.addWrapper(cereal.VulkanEncoder, "VkEncoder")
        self.addWrapper(cereal.VulkanMarshaling, "goldfish_vk_marshaling_guest")
        self.addWrapper(cereal.VulkanMarshaling, "goldfish_vk_marshaling")
        self.addWrapper(cereal.VulkanTesting, "goldfish_vk_testing")
        self.addWrapper(cereal.VulkanDeepcopy, "goldfish_vk_deepcopy")
        self.addWrapper(cereal.VulkanHandleMap, "goldfish_vk_handlemap")
        self.addWrapper(cereal.VulkanDispatch, "goldfish_vk_dispatch")
        self.addWrapper(cereal.VulkanDecoder, "VkDecoder")

        self.guestAndroidMkCppFiles = ""
        self.hostCMakeCppFiles = ""
        self.hostDecoderCMakeCppFiles = ""

        def addSrcEntry(m):
            mkSrcEntry = m.getMakefileSrcEntry()
            cmakeSrcEntry = m.getCMakeSrcEntry()
            if m.directory == self.guest_tag:
                self.guestAndroidMkCppFiles += mkSrcEntry
            elif m.directory == self.host_tag:
                self.hostDecoderCMakeCppFiles += cmakeSrcEntry
            else:
                self.hostCMakeCppFiles += cmakeSrcEntry

        self.forEachModule(addSrcEntry)

    def addGuestModule(self, basename, extraHeader = "", extraImpl = "", useNamespace = True):
        self.addModule(self.guest_tag,
                       basename,
                       extraHeader = extraHeader,
                       extraImpl = extraImpl,
                       customAbsDir = self.guest_abs_destination,
                       useNamespace = useNamespace)

    def addHostModule(self, basename, extraHeader = "", extraImpl = "", useNamespace = True):
        self.addModule(self.host_tag,
                       basename,
                       extraHeader = extraHeader,
                       extraImpl = extraImpl,
                       customAbsDir = self.host_abs_decoder_destination,
                       useNamespace = useNamespace)

    def addModule(self, directory, basename,
                  extraHeader = "", extraImpl = "",
                  customAbsDir = None,
                  useNamespace = True):
        self.moduleList.append(basename)
        self.modules[basename] = \
            cereal.Module(directory, basename, customAbsDir = customAbsDir)
        self.modules[basename].headerPreamble = copyrightHeader
        self.modules[basename].headerPreamble += \
                autogeneratedHeaderTemplate % \
                (basename, "(header) generated by %s" % banner_command(sys.argv))

        namespaceBegin = "namespace goldfish_vk {" if useNamespace else ""
        namespaceEnd = "} // namespace goldfish_vk" if useNamespace else ""

        self.modules[basename].headerPreamble += """
#pragma once

#include <vulkan/vulkan.h>

%s

%s

""" % (extraHeader, namespaceBegin)

        self.modules[basename].implPreamble = copyrightHeader
        self.modules[basename].implPreamble += \
                autogeneratedHeaderTemplate % \
                (basename, "(impl) generated by %s" % \
                    banner_command(sys.argv))
        self.modules[basename].implPreamble += """
#include "%s.h"

%s

%s

""" % (basename, extraImpl, namespaceBegin)

        self.modules[basename].headerPostamble = """
%s
""" % namespaceEnd
        self.modules[basename].implPostamble = """
%s
""" % namespaceEnd

    def addWrapper(self, moduleType, moduleName):
        self.wrappers.append( \
            moduleType( \
                self.modules[moduleName],
                self.typeInfo))

    def forEachModule(self, func):
        for moduleName in self.moduleList:
            func(self.modules[moduleName])

    def forEachWrapper(self, func):
        for wrapper in self.wrappers:
            func(wrapper)

## Overrides####################################################################

    def beginFile(self, genOpts):
        OutputGenerator.beginFile(self, genOpts)

        write(self.host_cmake_generator(self.hostCMakeCppFiles),
              file = self.outFile)

        guestAndroidMkPath = \
            os.path.join( \
                self.guest_abs_destination,
                "Android.mk")

        guestAndroidMkFile = open(guestAndroidMkPath, "w", encoding="utf-8")

        write(self.guest_android_mk_generator(self.guestAndroidMkCppFiles),
              file = guestAndroidMkFile)

        guestAndroidMkFile.close()

        self.forEachModule(lambda m: m.begin(self.genOpts.directory))
        self.forEachWrapper(lambda w: w.onBegin())

    def endFile(self):
        OutputGenerator.endFile(self)

        self.forEachWrapper(lambda w: w.onEnd())
        self.forEachModule(lambda m: m.end())

    def beginFeature(self, interface, emit):
        # Start processing in superclass
        OutputGenerator.beginFeature(self, interface, emit)

        self.forEachModule(lambda m: m.appendHeader("#ifdef %s\n" % self.featureName))
        self.forEachModule(lambda m: m.appendImpl("#ifdef %s\n" % self.featureName))
        self.forEachWrapper(lambda w: w.onBeginFeature(self.featureName))

    def endFeature(self):
        # Finish processing in superclass
        OutputGenerator.endFeature(self)

        self.forEachModule(lambda m: m.appendHeader("#endif\n"))
        self.forEachModule(lambda m: m.appendImpl("#endif\n"))
        self.forEachWrapper(lambda w: w.onEndFeature())

    def genType(self, typeinfo, name, alias):
        OutputGenerator.genType(self, typeinfo, name, alias)
        self.typeInfo.onGenType(typeinfo, name, alias)
        self.forEachWrapper(lambda w: w.onGenType(typeinfo, name, alias))

    def genStruct(self, typeinfo, typeName, alias):
        OutputGenerator.genStruct(self, typeinfo, typeName, alias)
        self.typeInfo.onGenStruct(typeinfo, typeName, alias)
        self.forEachWrapper(lambda w: w.onGenStruct(typeinfo, typeName, alias))

    def genGroup(self, groupinfo, groupName, alias = None):
        OutputGenerator.genGroup(self, groupinfo, groupName, alias)
        self.typeInfo.onGenGroup(groupinfo, groupName, alias)
        self.forEachWrapper(lambda w: w.onGenGroup(groupinfo, groupName, alias))

    def genEnum(self, enuminfo, name, alias):
        OutputGenerator.genEnum(self, enuminfo, name, alias)
        self.typeInfo.onGenEnum(enuminfo, name, alias)
        self.forEachWrapper(lambda w: w.onGenEnum(enuminfo, name, alias))

    def genCmd(self, cmdinfo, name, alias):
        OutputGenerator.genCmd(self, cmdinfo, name, alias)
        self.typeInfo.onGenCmd(cmdinfo, name, alias)
        self.forEachWrapper(lambda w: w.onGenCmd(cmdinfo, name, alias))
