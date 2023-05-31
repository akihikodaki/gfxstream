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
from pathlib import Path, PurePosixPath

import cereal
from cereal.wrapperdefs import VULKAN_STREAM_TYPE
from cereal.wrapperdefs import VULKAN_STREAM_TYPE_GUEST

# CerealGenerator - generates set of driver sources
# while being agnostic to the stream implementation
from reg import GroupInfo, TypeInfo, EnumInfo

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

# We put the long generated commands in a separate paragraph, so that the formatter won't mess up
# with other texts.
autogeneratedHeaderTemplate = """
// Autogenerated module %s
//
// %s
//
// Please do not modify directly;
// re-run gfxstream-protocols/scripts/generate-vulkan-sources.sh,
// or directly from Python by defining:
// VULKAN_REGISTRY_XML_DIR : Directory containing vk.xml
// VULKAN_REGISTRY_SCRIPTS_DIR : Directory containing genvk.py
// CEREAL_OUTPUT_DIR: Where to put the generated sources.
//
// python3 $VULKAN_REGISTRY_SCRIPTS_DIR/genvk.py -registry $VULKAN_REGISTRY_XML_DIR/vk.xml cereal -o $CEREAL_OUTPUT_DIR
//
"""

autogeneratedMkTemplate = """
# Autogenerated makefile
# %s
# Please do not modify directly;
# re-run gfxstream-protocols/scripts/generate-vulkan-sources.sh,
# or directly from Python by defining:
# VULKAN_REGISTRY_XML_DIR : Directory containing vk.xml
# VULKAN_REGISTRY_SCRIPTS_DIR : Directory containing genvk.py
# CEREAL_OUTPUT_DIR: Where to put the generated sources.
# python3 $VULKAN_REGISTRY_SCRIPTS_DIR/genvk.py -registry $VULKAN_REGISTRY_XML_DIR/vk.xml cereal -o $CEREAL_OUTPUT_DIR
"""

def banner_command(argv):
    """Return sanitized command-line description.
       |argv| must be a list of command-line parameters, e.g. sys.argv.
       Return a string corresponding to the command, with platform-specific
       paths removed."""

    def makePosixRelative(someArg):
        if os.path.exists(someArg):
            return str(PurePosixPath(Path(os.path.relpath(someArg))))
        return someArg

    return ' '.join(map(makePosixRelative, argv))

suppressEnabled = False
suppressExceptModule = None

def envGetOrDefault(key, default=None):
    if key in os.environ:
        return os.environ[key]
    print("envGetOrDefault: notfound: %s" % key)
    return default

def init_suppress_option():
    global suppressEnabled
    global suppressExceptModule

    if "ANDROID_EMU_VK_CEREAL_SUPPRESS" in os.environ:
        option = os.environ["ANDROID_EMU_VK_CEREAL_SUPPRESS"]

        if option != "":
            suppressExceptModule = option
            suppressEnabled = True
            print("suppressEnabled: %s" % suppressExceptModule)

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

        init_suppress_option()

        self.typeInfo = cereal.VulkanTypeInfo(self)

        self.modules = {}
        self.protos = {}
        self.moduleList = []
        self.protoList = []

        self.wrappers = []

        self.codegen = cereal.CodeGen()

        self.guestBaseLibDirPrefix = \
            envGetOrDefault("VK_CEREAL_GUEST_BASELIB_PREFIX", "aemu/base")
        self.baseLibDirPrefix = \
            envGetOrDefault("VK_CEREAL_BASELIB_PREFIX", "aemu/base")
        self.baseLibLinkName = \
            envGetOrDefault("VK_CEREAL_BASELIB_LINKNAME", "android-emu-base")
        self.vulkanHeaderTargetName = envGetOrDefault("VK_CEREAL_VK_HEADER_TARGET", "")
        self.utilsHeader = envGetOrDefault("VK_CEREAL_UTILS_LINKNAME", "")
        self.utilsHeaderDirPrefix = envGetOrDefault("VK_CEREAL_UTILS_PREFIX", "utils")

        # THe host always needs all possible guest struct definitions, while the guest only needs
        # platform sepcific headers.
        self.hostCommonExtraVulkanHeaders = '#include "vk_android_native_buffer.h"'
        self.host_cmake_generator = lambda cppFiles: f"""{autogeneratedMkTemplate % banner_command(sys.argv)}
add_library(OpenglRender_vulkan_cereal {cppFiles})
target_compile_definitions(OpenglRender_vulkan_cereal PRIVATE -DVK_GOOGLE_gfxstream)
if (WIN32)
    target_compile_definitions(OpenglRender_vulkan_cereal PRIVATE -DVK_USE_PLATFORM_WIN32_KHR)
endif()
target_link_libraries(
    OpenglRender_vulkan_cereal
    PUBLIC
    {self.baseLibLinkName}
    {self.vulkanHeaderTargetName}
    PRIVATE
    {self.utilsHeader})

target_include_directories(OpenglRender_vulkan_cereal
                           PUBLIC
                           .
                           PRIVATE
                           ..
                           ../..
                           ../../../include)
"""

        encoderInclude = f"""
#include "{self.guestBaseLibDirPrefix}/AndroidHealthMonitor.h"
#include "goldfish_vk_private_defs.h"
#include <memory>

namespace gfxstream {{
class IOStream;
}}
"""
        encoderImplInclude = f"""
#include "EncoderDebug.h"
#include "IOStream.h"
#include "Resources.h"
#include "ResourceTracker.h"
#include "Validation.h"
#include "%s.h"

#include "{self.guestBaseLibDirPrefix}/AlignedBuf.h"
#include "{self.guestBaseLibDirPrefix}/BumpPool.h"
#include "{self.guestBaseLibDirPrefix}/synchronization/AndroidLock.h"

#include <cutils/properties.h>

#include "goldfish_vk_marshaling_guest.h"
#include "goldfish_vk_reserved_marshaling_guest.h"
#include "goldfish_vk_deepcopy_guest.h"
#include "goldfish_vk_counting_guest.h"
#include "goldfish_vk_handlemap_guest.h"
#include "goldfish_vk_private_defs.h"
#include "goldfish_vk_transform_guest.h"

#include <memory>
#include <optional>
#include <unordered_map>
#include <string>
#include <vector>

""" % VULKAN_STREAM_TYPE_GUEST

        functableImplInclude = """
#include "VkEncoder.h"
#include "../OpenglSystemCommon/HostConnection.h"
#include "ResourceTracker.h"

#include "goldfish_vk_private_defs.h"

#include <log/log.h>
#include <cstring>

// Stuff we are not going to use but if included,
// will cause compile errors. These are Android Vulkan
// required extensions, but the approach will be to
// implement them completely on the guest side.
#undef VK_KHR_android_surface
"""
        marshalIncludeGuest = """
#include "goldfish_vk_marshaling_guest.h"
#include "goldfish_vk_private_defs.h"
#include "%s.h"

// Stuff we are not going to use but if included,
// will cause compile errors. These are Android Vulkan
// required extensions, but the approach will be to
// implement them completely on the guest side.
#undef VK_KHR_android_surface
#undef VK_ANDROID_external_memory_android_hardware_buffer
""" % VULKAN_STREAM_TYPE_GUEST

        reservedmarshalIncludeGuest = """
#include "goldfish_vk_marshaling_guest.h"
#include "goldfish_vk_private_defs.h"
#include "%s.h"

// Stuff we are not going to use but if included,
// will cause compile errors. These are Android Vulkan
// required extensions, but the approach will be to
// implement them completely on the guest side.
#undef VK_KHR_android_surface
#undef VK_ANDROID_external_memory_android_hardware_buffer
""" % VULKAN_STREAM_TYPE_GUEST

        reservedmarshalImplIncludeGuest = """
#include "Resources.h"
"""

        vulkanStreamIncludeHost = f"""
{self.hostCommonExtraVulkanHeaders}
#include "goldfish_vk_private_defs.h"

#include "%s.h"
#include "{self.baseLibDirPrefix}/files/StreamSerializing.h"
""" % VULKAN_STREAM_TYPE

        testingInclude = f"""
{self.hostCommonExtraVulkanHeaders}
#include "goldfish_vk_private_defs.h"
#include <string.h>
#include <functional>
using OnFailCompareFunc = std::function<void(const char*)>;
"""
        poolInclude = f"""
{self.hostCommonExtraVulkanHeaders}
#include "goldfish_vk_private_defs.h"
#include "{self.baseLibDirPrefix}/BumpPool.h"
using android::base::Allocator;
using android::base::BumpPool;
"""
        handleMapInclude = f"""
{self.hostCommonExtraVulkanHeaders}
#include "goldfish_vk_private_defs.h"
#include "VulkanHandleMapping.h"
"""
        transformIncludeGuest = """
#include "goldfish_vk_private_defs.h"
"""
        transformInclude = f"""
{self.hostCommonExtraVulkanHeaders}
#include "goldfish_vk_private_defs.h"
#include "goldfish_vk_extension_structs.h"
"""
        transformImplIncludeGuest = """
#include "ResourceTracker.h"
"""
        transformImplInclude = """
#include "VkDecoderGlobalState.h"
"""
        deepcopyInclude = """
#include "vk_util.h"
"""
        poolIncludeGuest = f"""
#include "goldfish_vk_private_defs.h"
#include "{self.guestBaseLibDirPrefix}/BumpPool.h"
using android::base::Allocator;
using android::base::BumpPool;
// Stuff we are not going to use but if included,
// will cause compile errors. These are Android Vulkan
// required extensions, but the approach will be to
// implement them completely on the guest side.
#undef VK_KHR_android_surface
#undef VK_ANDROID_external_memory_android_hardware_buffer
"""
        handleMapIncludeGuest = """
#include "goldfish_vk_private_defs.h"
#include "VulkanHandleMapping.h"
// Stuff we are not going to use but if included,
// will cause compile errors. These are Android Vulkan
// required extensions, but the approach will be to
// implement them completely on the guest side.
#undef VK_KHR_android_surface
#undef VK_ANDROID_external_memory_android_hardware_buffer
"""
        dispatchHeaderDefs = f"""
{self.hostCommonExtraVulkanHeaders}
#include "goldfish_vk_private_defs.h"
namespace gfxstream {{
namespace vk {{

struct VulkanDispatch;

}} // namespace vk
}} // namespace gfxstream
using DlOpenFunc = void* (void);
using DlSymFunc = void* (void*, const char*);
"""

        extensionStructsInclude = f"""
{self.hostCommonExtraVulkanHeaders}
#include "goldfish_vk_private_defs.h"
"""

        extensionStructsIncludeGuest = """
#include "vk_platform_compat.h"
#include "goldfish_vk_private_defs.h"
// Stuff we are not going to use but if included,
// will cause compile errors. These are Android Vulkan
// required extensions, but the approach will be to
// implement them completely on the guest side.
#undef VK_KHR_android_surface
#undef VK_ANDROID_external_memory_android_hardware_buffer
"""
        commonCerealImplIncludes = """
#include "goldfish_vk_extension_structs.h"
#include "goldfish_vk_private_defs.h"
#include <string.h>
"""
        commonCerealIncludesGuest = """
#include "vk_platform_compat.h"
"""
        commonCerealImplIncludesGuest = """
#include "goldfish_vk_extension_structs_guest.h"
#include "goldfish_vk_private_defs.h"

#include <cstring>
"""
        countingIncludes = """
#include "vk_platform_compat.h"
#include "goldfish_vk_private_defs.h"
"""

        dispatchImplIncludes = """
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
"""

        decoderSnapshotHeaderIncludes = f"""
#include <memory>
#include "{self.utilsHeaderDirPrefix}/GfxApiLogger.h"
#include "{self.baseLibDirPrefix}/HealthMonitor.h"
#include "common/goldfish_vk_private_defs.h"
"""
        decoderSnapshotImplIncludes = f"""
#include "VulkanHandleMapping.h"
#include "VkDecoderGlobalState.h"
#include "VkReconstruction.h"

#include "{self.baseLibDirPrefix}/synchronization/Lock.h"
"""

        decoderHeaderIncludes = f"""
#include "VkDecoderContext.h"

#include <memory>

namespace android {{
namespace base {{
class BumpPool;
}} // namespace android
}} // namespace base

"""

        decoderImplIncludes = f"""
#include "common/goldfish_vk_marshaling.h"
#include "common/goldfish_vk_reserved_marshaling.h"
#include "common/goldfish_vk_private_defs.h"
#include "common/goldfish_vk_transform.h"

#include "{self.baseLibDirPrefix}/BumpPool.h"
#include "{self.baseLibDirPrefix}/system/System.h"
#include "{self.baseLibDirPrefix}/Tracing.h"
#include "{self.baseLibDirPrefix}/Metrics.h"
#include "render-utils/IOStream.h"
#include "host/FrameBuffer.h"
#include "host-common/feature_control.h"
#include "host-common/GfxstreamFatalError.h"
#include "host-common/logging.h"

#include "VkDecoderGlobalState.h"
#include "VkDecoderSnapshot.h"

#include "VulkanDispatch.h"
#include "%s.h"

#include <functional>
#include <optional>
#include <unordered_map>
""" % VULKAN_STREAM_TYPE

        def createVkExtensionStructureTypePreamble(extensionName: str) -> str:
            return f"""
#define {extensionName}_ENUM(type,id) \
    ((type)(1000000000 + (1000 * ({extensionName}_NUMBER - 1)) + (id)))
"""
        self.guest_encoder_tag = "guest_encoder"
        self.guest_hal_tag = "guest_hal"
        self.host_tag = "host"

        default_guest_abs_encoder_destination = \
            os.path.join(
                os.getcwd(),
                "..", "..",
                "device", "generic", "goldfish-opengl",
                "system", "vulkan_enc")
        self.guest_abs_encoder_destination = \
            envGetOrDefault("VK_CEREAL_GUEST_ENCODER_DIR",
                            default_guest_abs_encoder_destination)

        default_guest_abs_hal_destination = \
            os.path.join(
                os.getcwd(),
                "..", "..",
                "device", "generic", "goldfish-opengl",
                "system", "vulkan")
        self.guest_abs_hal_destination = \
            envGetOrDefault("VK_CEREAL_GUEST_HAL_DIR",
                            default_guest_abs_hal_destination)

        default_host_abs_decoder_destination = \
            os.path.join(
                os.getcwd(),
                "android", "android-emugl", "host",
                "libs", "libOpenglRender", "vulkan")
        self.host_abs_decoder_destination = \
            envGetOrDefault("VK_CEREAL_HOST_DECODER_DIR",
                            default_host_abs_decoder_destination)
        self.host_script_destination = envGetOrDefault("VK_CEREAL_HOST_SCRIPTS_DIR")
        assert(self.host_script_destination is not None)

        self.addGuestEncoderModule(
            "VkEncoder",
            extraHeader = encoderInclude,
            extraImpl = encoderImplInclude)

        self.addGuestEncoderModule("goldfish_vk_extension_structs_guest",
                                   extraHeader=extensionStructsIncludeGuest)
        self.addGuestEncoderModule("goldfish_vk_marshaling_guest",
                                   extraHeader=commonCerealIncludesGuest + marshalIncludeGuest,
                                   extraImpl=commonCerealImplIncludesGuest)
        self.addGuestEncoderModule("goldfish_vk_reserved_marshaling_guest",
                                   extraHeader=commonCerealIncludesGuest + reservedmarshalIncludeGuest,
                                   extraImpl=commonCerealImplIncludesGuest + reservedmarshalImplIncludeGuest)
        self.addGuestEncoderModule("goldfish_vk_deepcopy_guest",
                                   extraHeader=commonCerealIncludesGuest + poolIncludeGuest,
                                   extraImpl=commonCerealImplIncludesGuest + deepcopyInclude)
        self.addGuestEncoderModule("goldfish_vk_counting_guest",
                                   extraHeader=countingIncludes,
                                   extraImpl=commonCerealImplIncludesGuest)
        self.addGuestEncoderModule("goldfish_vk_handlemap_guest",
                                   extraHeader=commonCerealIncludesGuest + handleMapIncludeGuest,
                                   extraImpl=commonCerealImplIncludesGuest)
        self.addGuestEncoderModule("goldfish_vk_transform_guest",
                                   extraHeader=commonCerealIncludesGuest + transformIncludeGuest,
                                   extraImpl=commonCerealImplIncludesGuest + transformImplIncludeGuest)
        self.addGuestEncoderModule(
            "vulkan_gfxstream_structure_type", headerOnly=True, suppressFeatureGuards=True,
            moduleName="vulkan_gfxstream_structure_type_guest", useNamespace=False,
            suppressVulkanHeaders=True,
            extraHeader=createVkExtensionStructureTypePreamble('VK_GOOGLE_GFXSTREAM'))

        self.addGuestEncoderModule("func_table", extraImpl=functableImplInclude)

        self.addCppModule("common", "goldfish_vk_extension_structs",
                       extraHeader=extensionStructsInclude)
        self.addCppModule("common", "goldfish_vk_marshaling",
                       extraHeader=vulkanStreamIncludeHost,
                       extraImpl=commonCerealImplIncludes)
        self.addCppModule("common", "goldfish_vk_reserved_marshaling",
                       extraHeader=vulkanStreamIncludeHost,
                       extraImpl=commonCerealImplIncludes)
        self.addCppModule("common", "goldfish_vk_testing",
                       extraHeader=testingInclude,
                       extraImpl=commonCerealImplIncludes)
        self.addCppModule("common", "goldfish_vk_deepcopy",
                       extraHeader=poolInclude,
                       extraImpl=commonCerealImplIncludes + deepcopyInclude)
        self.addCppModule("common", "goldfish_vk_handlemap",
                       extraHeader=handleMapInclude,
                       extraImpl=commonCerealImplIncludes)
        self.addCppModule("common", "goldfish_vk_dispatch",
                       extraHeader=dispatchHeaderDefs,
                       extraImpl=dispatchImplIncludes)
        self.addCppModule("common", "goldfish_vk_transform",
                       extraHeader=transformInclude,
                       extraImpl=transformImplInclude)
        self.addHostModule("VkDecoder",
                           extraHeader=decoderHeaderIncludes,
                           extraImpl=decoderImplIncludes,
                           useNamespace=False)
        self.addHostModule("VkDecoderSnapshot",
                           extraHeader=decoderSnapshotHeaderIncludes,
                           extraImpl=decoderSnapshotImplIncludes,
                           useNamespace=False)
        self.addHostModule("VkSubDecoder",
                           extraHeader="",
                           extraImpl="",
                           useNamespace=False,
                           implOnly=True)

        self.addModule(cereal.PyScript(self.host_tag, "vulkan_printer", customAbsDir=Path(
            self.host_script_destination) / "print_gfx_logs"), moduleName="ApiLogDecoder")
        self.addHostModule(
            "vulkan_gfxstream_structure_type", headerOnly=True, suppressFeatureGuards=True,
            moduleName="vulkan_gfxstream_structure_type_host", useNamespace=False,
            suppressVulkanHeaders=True,
            extraHeader=createVkExtensionStructureTypePreamble('VK_GOOGLE_GFXSTREAM'))
        self.addHostModule(
            "vk_android_native_buffer_structure_type", headerOnly=True, suppressFeatureGuards=True,
            useNamespace=False, suppressVulkanHeaders=True,
            extraHeader=createVkExtensionStructureTypePreamble('VK_ANDROID_NATIVE_BUFFER'))

        self.addWrapper(cereal.VulkanEncoder, "VkEncoder")
        self.addWrapper(cereal.VulkanExtensionStructs, "goldfish_vk_extension_structs_guest")
        self.addWrapper(cereal.VulkanMarshaling, "goldfish_vk_marshaling_guest", variant = "guest")
        self.addWrapper(cereal.VulkanReservedMarshaling, "goldfish_vk_reserved_marshaling_guest", variant = "guest")
        self.addWrapper(cereal.VulkanDeepcopy, "goldfish_vk_deepcopy_guest")
        self.addWrapper(cereal.VulkanCounting, "goldfish_vk_counting_guest")
        self.addWrapper(cereal.VulkanHandleMap, "goldfish_vk_handlemap_guest")
        self.addWrapper(cereal.VulkanTransform, "goldfish_vk_transform_guest")
        self.addWrapper(cereal.VulkanFuncTable, "func_table")
        self.addWrapper(cereal.VulkanExtensionStructs, "goldfish_vk_extension_structs")
        self.addWrapper(cereal.VulkanMarshaling, "goldfish_vk_marshaling")
        self.addWrapper(cereal.VulkanReservedMarshaling, "goldfish_vk_reserved_marshaling", variant = "host")
        self.addWrapper(cereal.VulkanTesting, "goldfish_vk_testing")
        self.addWrapper(cereal.VulkanDeepcopy, "goldfish_vk_deepcopy")
        self.addWrapper(cereal.VulkanHandleMap, "goldfish_vk_handlemap")
        self.addWrapper(cereal.VulkanDispatch, "goldfish_vk_dispatch")
        self.addWrapper(cereal.VulkanTransform, "goldfish_vk_transform", resourceTrackerTypeName="VkDecoderGlobalState")
        self.addWrapper(cereal.VulkanDecoder, "VkDecoder")
        self.addWrapper(cereal.VulkanDecoderSnapshot, "VkDecoderSnapshot")
        self.addWrapper(cereal.VulkanSubDecoder, "VkSubDecoder")
        self.addWrapper(cereal.ApiLogDecoder, "ApiLogDecoder")
        self.addWrapper(cereal.VulkanGfxstreamStructureType,
                        "vulkan_gfxstream_structure_type_guest")
        self.addWrapper(cereal.VulkanGfxstreamStructureType, "vulkan_gfxstream_structure_type_host")
        self.addWrapper(cereal.VulkanAndroidNativeBufferStructureType,
                        "vk_android_native_buffer_structure_type")

        self.guestAndroidMkCppFiles = ""
        self.hostCMakeCppFiles = ""
        self.hostDecoderCMakeCppFiles = ""

        def addSrcEntry(m):
            mkSrcEntry = m.getMakefileSrcEntry()
            cmakeSrcEntry = m.getCMakeSrcEntry()
            if m.directory == self.guest_encoder_tag:
                self.guestAndroidMkCppFiles += mkSrcEntry
            elif m.directory == self.host_tag:
                self.hostDecoderCMakeCppFiles += cmakeSrcEntry
            elif m.directory != self.guest_hal_tag:
                self.hostCMakeCppFiles += cmakeSrcEntry

        self.forEachModule(addSrcEntry)

    def addGuestEncoderModule(
            self, basename, extraHeader="", extraImpl="", useNamespace=True, headerOnly=False,
            suppressFeatureGuards=False, moduleName=None, suppressVulkanHeaders=False):
        if not os.path.exists(self.guest_abs_encoder_destination):
            print("Path [%s] not found (guest encoder path), skipping" % self.guest_abs_encoder_destination)
            return
        self.addCppModule(self.guest_encoder_tag, basename, extraHeader=extraHeader,
                       extraImpl=extraImpl, customAbsDir=self.guest_abs_encoder_destination,
                       useNamespace=useNamespace, headerOnly=headerOnly,
                       suppressFeatureGuards=suppressFeatureGuards, moduleName=moduleName,
                       suppressVulkanHeaders=suppressVulkanHeaders)

    def addGuestHalModule(self, basename, extraHeader = "", extraImpl = "", useNamespace = True):
        if not os.path.exists(self.guest_abs_hal_destination):
            print("Path [%s] not found (guest encoder path), skipping" % self.guest_abs_encoder_destination)
            return
        self.addCppModule(self.guest_hal_tag,
                       basename,
                       extraHeader = extraHeader,
                       extraImpl = extraImpl,
                       customAbsDir = self.guest_abs_hal_destination,
                       useNamespace = useNamespace)

    def addHostModule(
            self, basename, extraHeader="", extraImpl="", useNamespace=True, implOnly=False,
            suppress=False, headerOnly=False, suppressFeatureGuards=False, moduleName=None,
            suppressVulkanHeaders=False):
        if not os.path.exists(self.host_abs_decoder_destination):
            print("Path [%s] not found (host encoder path), skipping" %
                  self.host_abs_decoder_destination)
            return
        if not suppressVulkanHeaders:
            extraHeader = self.hostCommonExtraVulkanHeaders + '\n' + extraHeader
        self.addCppModule(
            self.host_tag, basename, extraHeader=extraHeader, extraImpl=extraImpl,
            customAbsDir=self.host_abs_decoder_destination, useNamespace=useNamespace,
            implOnly=implOnly, suppress=suppress, headerOnly=headerOnly,
            suppressFeatureGuards=suppressFeatureGuards, moduleName=moduleName,
            suppressVulkanHeaders=suppressVulkanHeaders)

    def addModule(self, module, moduleName=None):
        if moduleName is None:
            moduleName = module.basename
        self.moduleList.append(moduleName)
        self.modules[moduleName] = module

    def addCppModule(
            self, directory, basename, extraHeader="", extraImpl="", customAbsDir=None,
            useNamespace=True, implOnly=False, suppress=False, headerOnly=False,
            suppressFeatureGuards=False, moduleName=None, suppressVulkanHeaders=False):
        module = cereal.Module(
            directory, basename, customAbsDir=customAbsDir, suppress=suppress, implOnly=implOnly,
            headerOnly=headerOnly, suppressFeatureGuards=suppressFeatureGuards)
        self.addModule(module, moduleName=moduleName)
        module.headerPreamble = copyrightHeader
        module.headerPreamble += \
                autogeneratedHeaderTemplate % \
                (basename, "(header) generated by %s" % banner_command(sys.argv))


        namespaceBegin = """
namespace gfxstream {
namespace vk {
""" if useNamespace else ""

        namespaceEnd = """
}  // namespace vk"
}  // namespace gfxstream
""" if useNamespace else ""

        module.headerPreamble += "#pragma once\n"
        if (not suppressVulkanHeaders):
            module.headerPreamble += "#include <vulkan/vulkan.h>\n"
            module.headerPreamble += '#include "vulkan_gfxstream.h"\n'
        module.headerPreamble += extraHeader + '\n'
        if namespaceBegin:
            module.headerPreamble += namespaceBegin + '\n'

        module.implPreamble = copyrightHeader
        module.implPreamble += \
                autogeneratedHeaderTemplate % \
                (basename, "(impl) generated by %s" % \
                    banner_command(sys.argv))
        if not implOnly:
            module.implPreamble += """
#include "%s.h"

%s

%s

""" % (basename, extraImpl, namespaceBegin)

        module.headerPostamble = """
%s
""" % namespaceEnd
        module.implPostamble = """
%s
""" % namespaceEnd

    def addWrapper(self, moduleType, moduleName, **kwargs):
        if moduleName not in self.modules:
            print(f'Unknown module: {moduleName}. All known modules are: {", ".join(self.modules)}.')
            return
        self.wrappers.append(
            moduleType(
                self.modules[moduleName],
                self.typeInfo, **kwargs))

    def forEachModule(self, func):
        for moduleName in self.moduleList:
            func(self.modules[moduleName])

    def forEachWrapper(self, func):
        for wrapper in self.wrappers:
            func(wrapper)

## Overrides####################################################################

    def beginFile(self, genOpts):
        OutputGenerator.beginFile(self, genOpts, suppressEnabled)

        if suppressEnabled:
            def enableSuppression(m):
                m.suppress = True
            self.forEachModule(enableSuppression)
            self.modules[suppressExceptModule].suppress = False

        if not suppressEnabled:
            write(self.host_cmake_generator(self.hostCMakeCppFiles),
                  file = self.outFile)

            guestEncoderAndroidMkPath = \
                os.path.join( \
                    self.guest_abs_encoder_destination,
                    "Android.mk")

        self.forEachModule(lambda m: m.begin(self.genOpts.directory))
        self.forEachWrapper(lambda w: w.onBegin())

    def endFile(self):
        OutputGenerator.endFile(self)

        self.typeInfo.onEnd()

        self.forEachWrapper(lambda w: w.onEnd())
        self.forEachModule(lambda m: m.end())

    def beginFeature(self, interface, emit):
        # Start processing in superclass
        OutputGenerator.beginFeature(self, interface, emit)

        self.typeInfo.onBeginFeature(self.featureName, self.featureType)

        self.forEachModule(
            lambda m: m.appendHeader("#ifdef %s\n" % self.featureName)
            if isinstance(m, cereal.Module) and not m.suppressFeatureGuards else None)
        self.forEachModule(
            lambda m: m.appendImpl("#ifdef %s\n" % self.featureName)
            if isinstance(m, cereal.Module) and not m.suppressFeatureGuards else None)
        self.forEachWrapper(lambda w: w.onBeginFeature(self.featureName, self.featureType))
        # functable needs to understand the feature type (device vs instance) of each cmd
        for features in interface.findall('require'):
            for c in features.findall('command'):
                self.forEachWrapper(lambda w: w.onFeatureNewCmd(c.get('name')))

    def endFeature(self):
        # Finish processing in superclass
        OutputGenerator.endFeature(self)

        self.typeInfo.onEndFeature()

        self.forEachModule(lambda m: m.appendHeader("#endif\n") if isinstance(
            m, cereal.Module) and not m.suppressFeatureGuards else None)
        self.forEachModule(lambda m: m.appendImpl("#endif\n") if isinstance(
            m, cereal.Module) and not m.suppressFeatureGuards else None)
        self.forEachWrapper(lambda w: w.onEndFeature())

    def genType(self, typeinfo: TypeInfo, name, alias):
        OutputGenerator.genType(self, typeinfo, name, alias)
        self.typeInfo.onGenType(typeinfo, name, alias)
        self.forEachWrapper(lambda w: w.onGenType(typeinfo, name, alias))

    def genStruct(self, typeinfo, typeName, alias):
        OutputGenerator.genStruct(self, typeinfo, typeName, alias)
        self.typeInfo.onGenStruct(typeinfo, typeName, alias)
        self.forEachWrapper(lambda w: w.onGenStruct(typeinfo, typeName, alias))

    def genGroup(self, groupinfo: GroupInfo, groupName, alias = None):
        OutputGenerator.genGroup(self, groupinfo, groupName, alias)
        self.typeInfo.onGenGroup(groupinfo, groupName, alias)
        self.forEachWrapper(lambda w: w.onGenGroup(groupinfo, groupName, alias))

    def genEnum(self, enuminfo: EnumInfo, name, alias):
        OutputGenerator.genEnum(self, enuminfo, name, alias)
        self.typeInfo.onGenEnum(enuminfo, name, alias)
        self.forEachWrapper(lambda w: w.onGenEnum(enuminfo, name, alias))

    def genCmd(self, cmdinfo, name, alias):
        OutputGenerator.genCmd(self, cmdinfo, name, alias)
        self.typeInfo.onGenCmd(cmdinfo, name, alias)
        self.forEachWrapper(lambda w: w.onGenCmd(cmdinfo, name, alias))
