// Copyright (C) 2018 The Android Open Source Project
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
#include <hardware/hwvulkan.h>

#include <log/log.h>

#include <errno.h>
#include <string.h>
#ifdef VK_USE_PLATFORM_FUCHSIA
#include <fidl/fuchsia.logger/cpp/wire.h>
#include <lib/syslog/global.h>
#include <lib/zx/channel.h>
#include <lib/zx/socket.h>
#include <lib/zxio/zxio.h>
#include <unistd.h>

#include "TraceProviderFuchsia.h"
#include "services/service_connector.h"
#endif

#include "HostConnection.h"
#include "ProcessPipe.h"
#include "ResourceTracker.h"
#include "VkEncoder.h"
#include "func_table.h"

// Used when there is no Vulkan support on the host.
// Copied from frameworks/native/vulkan/libvulkan/stubhal.cpp
namespace vkstubhal {

[[noreturn]] VKAPI_ATTR void NoOp() {
    LOG_ALWAYS_FATAL("invalid stub function called");
}

VkResult
EnumerateInstanceExtensionProperties(const char* /*layer_name*/,
                                     uint32_t* count,
                                     VkExtensionProperties* /*properties*/) {
    AEMU_SCOPED_TRACE("vkstubhal::EnumerateInstanceExtensionProperties");
    *count = 0;
    return VK_SUCCESS;
}

VkResult
EnumerateInstanceLayerProperties(uint32_t* count,
                                 VkLayerProperties* /*properties*/) {
    AEMU_SCOPED_TRACE("vkstubhal::EnumerateInstanceLayerProperties");
    *count = 0;
    return VK_SUCCESS;
}

VkResult CreateInstance(const VkInstanceCreateInfo* /*create_info*/,
                        const VkAllocationCallbacks* /*allocator*/,
                        VkInstance* instance) {
    AEMU_SCOPED_TRACE("vkstubhal::CreateInstance");
    auto dispatch = new hwvulkan_dispatch_t;
    dispatch->magic = HWVULKAN_DISPATCH_MAGIC;
    *instance = reinterpret_cast<VkInstance>(dispatch);
    return VK_SUCCESS;
}

void DestroyInstance(VkInstance instance,
                     const VkAllocationCallbacks* /*allocator*/) {
    AEMU_SCOPED_TRACE("vkstubhal::DestroyInstance");
    auto dispatch = reinterpret_cast<hwvulkan_dispatch_t*>(instance);
    ALOG_ASSERT(dispatch->magic == HWVULKAN_DISPATCH_MAGIC,
                "DestroyInstance: invalid instance handle");
    delete dispatch;
}

VkResult EnumeratePhysicalDevices(VkInstance /*instance*/,
                                  uint32_t* count,
                                  VkPhysicalDevice* /*gpus*/) {
    AEMU_SCOPED_TRACE("vkstubhal::EnumeratePhysicalDevices");
    *count = 0;
    return VK_SUCCESS;
}

VkResult EnumerateInstanceVersion(uint32_t* pApiVersion) {
    AEMU_SCOPED_TRACE("vkstubhal::EnumerateInstanceVersion");
    *pApiVersion = VK_API_VERSION_1_0;
    return VK_SUCCESS;
}

VkResult
EnumeratePhysicalDeviceGroups(VkInstance /*instance*/,
                              uint32_t* count,
                              VkPhysicalDeviceGroupProperties* /*properties*/) {
    AEMU_SCOPED_TRACE("vkstubhal::EnumeratePhysicalDeviceGroups");
    *count = 0;
    return VK_SUCCESS;
}

VkResult
CreateDebugReportCallbackEXT(VkInstance /*instance*/,
                             const VkDebugReportCallbackCreateInfoEXT* /*pCreateInfo*/,
                             const VkAllocationCallbacks* /*pAllocator*/,
                             VkDebugReportCallbackEXT* pCallback)
{
    AEMU_SCOPED_TRACE("vkstubhal::CreateDebugReportCallbackEXT");
    *pCallback = VK_NULL_HANDLE;
    return VK_SUCCESS;
}

void
DestroyDebugReportCallbackEXT(VkInstance /*instance*/,
                              VkDebugReportCallbackEXT /*callback*/,
                              const VkAllocationCallbacks* /*pAllocator*/)
{
    AEMU_SCOPED_TRACE("vkstubhal::DestroyDebugReportCallbackEXT");
}

void
DebugReportMessageEXT(VkInstance /*instance*/,
                      VkDebugReportFlagsEXT /*flags*/,
                      VkDebugReportObjectTypeEXT /*objectType*/,
                      uint64_t /*object*/,
                      size_t /*location*/,
                      int32_t /*messageCode*/,
                      const char* /*pLayerPrefix*/,
                      const char* /*pMessage*/)
{
    AEMU_SCOPED_TRACE("vkstubhal::DebugReportMessageEXT");
}

VkResult
CreateDebugUtilsMessengerEXT(VkInstance /*instance*/,
                             const VkDebugUtilsMessengerCreateInfoEXT* /*pCreateInfo*/,
                             const VkAllocationCallbacks* /*pAllocator*/,
                             VkDebugUtilsMessengerEXT* pMessenger)
{
    AEMU_SCOPED_TRACE("vkstubhal::CreateDebugUtilsMessengerEXT");
    *pMessenger = VK_NULL_HANDLE;
    return VK_SUCCESS;
}

void
DestroyDebugUtilsMessengerEXT(VkInstance /*instance*/,
                              VkDebugUtilsMessengerEXT /*messenger*/,
                              const VkAllocationCallbacks* /*pAllocator*/)
{
    AEMU_SCOPED_TRACE("vkstubhal::DestroyDebugUtilsMessengerkEXT");
}

void
SubmitDebugUtilsMessageEXT(VkInstance /*instance*/,
                           VkDebugUtilsMessageSeverityFlagBitsEXT /*messageSeverity*/,
                           VkDebugUtilsMessageTypeFlagsEXT /*messageTypes*/,
                           const VkDebugUtilsMessengerCallbackDataEXT* /*pCallbackData*/)
{
    AEMU_SCOPED_TRACE("vkstubhal::SubmitDebugUtilsMessageEXT");
}

#ifdef VK_USE_PLATFORM_FUCHSIA
VkResult
GetMemoryZirconHandleFUCHSIA(VkDevice /*device*/,
                             const VkMemoryGetZirconHandleInfoFUCHSIA* /*pInfo*/,
                             uint32_t* pHandle) {
    AEMU_SCOPED_TRACE("vkstubhal::GetMemoryZirconHandleFUCHSIA");
    *pHandle = 0;
    return VK_SUCCESS;
}

VkResult
GetMemoryZirconHandlePropertiesFUCHSIA(VkDevice /*device*/,
                                       VkExternalMemoryHandleTypeFlagBits /*handleType*/,
                                       uint32_t /*handle*/,
                                       VkMemoryZirconHandlePropertiesFUCHSIA* /*pProperties*/) {
    AEMU_SCOPED_TRACE("vkstubhal::GetMemoryZirconHandlePropertiesFUCHSIA");
    return VK_SUCCESS;
}

VkResult
GetSemaphoreZirconHandleFUCHSIA(VkDevice /*device*/,
                                const VkSemaphoreGetZirconHandleInfoFUCHSIA* /*pInfo*/,
                                uint32_t* pHandle) {
    AEMU_SCOPED_TRACE("vkstubhal::GetSemaphoreZirconHandleFUCHSIA");
    *pHandle = 0;
    return VK_SUCCESS;
}

VkResult
ImportSemaphoreZirconHandleFUCHSIA(VkDevice /*device*/,
                                   const VkImportSemaphoreZirconHandleInfoFUCHSIA* /*pInfo*/) {
    AEMU_SCOPED_TRACE("vkstubhal::ImportSemaphoreZirconHandleFUCHSIA");
    return VK_SUCCESS;
}

VkResult CreateBufferCollectionFUCHSIA(
    VkDevice /*device*/,
    const VkBufferCollectionCreateInfoFUCHSIA* /*pInfo*/,
    const VkAllocationCallbacks* /*pAllocator*/,
    VkBufferCollectionFUCHSIA* /*pCollection*/) {
    AEMU_SCOPED_TRACE("vkstubhal::CreateBufferCollectionFUCHSIA");
    return VK_SUCCESS;
}

void DestroyBufferCollectionFUCHSIA(
    VkDevice /*device*/,
    VkBufferCollectionFUCHSIA /*collection*/,
    const VkAllocationCallbacks* /*pAllocator*/) {
    AEMU_SCOPED_TRACE("vkstubhal::DestroyBufferCollectionFUCHSIA");
}

VkResult SetBufferCollectionImageConstraintsFUCHSIA(
    VkDevice /*device*/,
    VkBufferCollectionFUCHSIA /*collection*/,
    const VkImageConstraintsInfoFUCHSIA* /*pImageConstraintsInfo*/) {
    AEMU_SCOPED_TRACE("vkstubhal::SetBufferCollectionImageConstraintsFUCHSIA");
    return VK_SUCCESS;
}

VkResult SetBufferCollectionBufferConstraintsFUCHSIA(
    VkDevice /*device*/,
    VkBufferCollectionFUCHSIA /*collection*/,
    const VkBufferConstraintsInfoFUCHSIA* /*pBufferConstraintsInfo*/) {
    AEMU_SCOPED_TRACE("vkstubhal::SetBufferCollectionBufferConstraintsFUCHSIA");
    return VK_SUCCESS;
}

VkResult GetBufferCollectionPropertiesFUCHSIA(
    VkDevice /*device*/,
    VkBufferCollectionFUCHSIA /*collection*/,
    VkBufferCollectionPropertiesFUCHSIA* /*pProperties*/) {
    AEMU_SCOPED_TRACE("vkstubhal::GetBufferCollectionPropertiesFUCHSIA");
    return VK_SUCCESS;
}
#endif

PFN_vkVoidFunction GetInstanceProcAddr(VkInstance instance,
                                       const char* name) {
    AEMU_SCOPED_TRACE("vkstubhal::GetInstanceProcAddr");
    if (strcmp(name, "vkCreateInstance") == 0)
        return reinterpret_cast<PFN_vkVoidFunction>(CreateInstance);
    if (strcmp(name, "vkDestroyInstance") == 0)
        return reinterpret_cast<PFN_vkVoidFunction>(DestroyInstance);
    if (strcmp(name, "vkEnumerateInstanceExtensionProperties") == 0)
        return reinterpret_cast<PFN_vkVoidFunction>(
            EnumerateInstanceExtensionProperties);
    if (strcmp(name, "vkEnumeratePhysicalDevices") == 0)
        return reinterpret_cast<PFN_vkVoidFunction>(EnumeratePhysicalDevices);
    if (strcmp(name, "vkEnumerateInstanceVersion") == 0)
        return reinterpret_cast<PFN_vkVoidFunction>(EnumerateInstanceVersion);
    if (strcmp(name, "vkEnumeratePhysicalDeviceGroups") == 0)
        return reinterpret_cast<PFN_vkVoidFunction>(
            EnumeratePhysicalDeviceGroups);
    if (strcmp(name, "vkEnumeratePhysicalDeviceGroupsKHR") == 0)
        return reinterpret_cast<PFN_vkVoidFunction>(
            EnumeratePhysicalDeviceGroups);
    if (strcmp(name, "vkGetInstanceProcAddr") == 0)
        return reinterpret_cast<PFN_vkVoidFunction>(GetInstanceProcAddr);
    if (strcmp(name, "vkCreateDebugReportCallbackEXT") == 0)
        return reinterpret_cast<PFN_vkVoidFunction>(CreateDebugReportCallbackEXT);
    if (strcmp(name, "vkDestroyDebugReportCallbackEXT") == 0)
        return reinterpret_cast<PFN_vkVoidFunction>(DestroyDebugReportCallbackEXT);
    if (strcmp(name, "vkDebugReportMessageEXT") == 0)
        return reinterpret_cast<PFN_vkVoidFunction>(DebugReportMessageEXT);
    if (strcmp(name, "vkCreateDebugUtilsMessengerEXT") == 0)
        return reinterpret_cast<PFN_vkVoidFunction>(CreateDebugUtilsMessengerEXT);
    if (strcmp(name, "vkDestroyDebugUtilsMessengerEXT") == 0)
        return reinterpret_cast<PFN_vkVoidFunction>(DestroyDebugUtilsMessengerEXT);
    if (strcmp(name, "vkSubmitDebugUtilsMessageEXT") == 0)
        return reinterpret_cast<PFN_vkVoidFunction>(SubmitDebugUtilsMessageEXT);
#ifdef VK_USE_PLATFORM_FUCHSIA
    if (strcmp(name, "vkGetMemoryZirconHandleFUCHSIA") == 0)
        return reinterpret_cast<PFN_vkVoidFunction>(GetMemoryZirconHandleFUCHSIA);
    if (strcmp(name, "vkGetMemoryZirconHandlePropertiesFUCHSIA") == 0)
        return reinterpret_cast<PFN_vkVoidFunction>(GetMemoryZirconHandlePropertiesFUCHSIA);
    if (strcmp(name, "vkGetSemaphoreZirconHandleFUCHSIA") == 0)
        return reinterpret_cast<PFN_vkVoidFunction>(GetSemaphoreZirconHandleFUCHSIA);
    if (strcmp(name, "vkImportSemaphoreZirconHandleFUCHSIA") == 0)
        return reinterpret_cast<PFN_vkVoidFunction>(ImportSemaphoreZirconHandleFUCHSIA);
    if (strcmp(name, "vkCreateBufferCollectionFUCHSIA") == 0)
        return reinterpret_cast<PFN_vkVoidFunction>(
            CreateBufferCollectionFUCHSIA);
    if (strcmp(name, "vkDestroyBufferCollectionFUCHSIA") == 0)
        return reinterpret_cast<PFN_vkVoidFunction>(
            DestroyBufferCollectionFUCHSIA);
    if (strcmp(name, "vkSetBufferCollectionImageConstraintsFUCHSIA") == 0)
        return reinterpret_cast<PFN_vkVoidFunction>(
            SetBufferCollectionImageConstraintsFUCHSIA);
    if (strcmp(name, "vkSetBufferCollectionBufferConstraintsFUCHSIA") == 0)
        return reinterpret_cast<PFN_vkVoidFunction>(
            SetBufferCollectionBufferConstraintsFUCHSIA);
    if (strcmp(name, "vkGetBufferCollectionPropertiesFUCHSIA") == 0)
        return reinterpret_cast<PFN_vkVoidFunction>(
            GetBufferCollectionPropertiesFUCHSIA);
#endif
    // Return NoOp for entrypoints that should never be called.
    if (strcmp(name, "vkGetPhysicalDeviceFeatures") == 0 ||
        strcmp(name, "vkGetPhysicalDeviceProperties") == 0 ||
        strcmp(name, "vkGetPhysicalDeviceFormatProperties") == 0 ||
        strcmp(name, "vkGetPhysicalDeviceImageFormatProperties") == 0 ||
        strcmp(name, "vkGetPhysicalDeviceMemoryProperties") == 0 ||
        strcmp(name, "vkGetPhysicalDeviceQueueFamilyProperties") == 0 ||
        strcmp(name, "vkGetDeviceProcAddr") == 0 ||
        strcmp(name, "vkCreateDevice") == 0 ||
        strcmp(name, "vkEnumerateDeviceExtensionProperties") == 0 ||
        strcmp(name, "vkGetPhysicalDeviceSparseImageFormatProperties") == 0 ||
        strcmp(name, "vkGetPhysicalDeviceFeatures2") == 0 ||
        strcmp(name, "vkGetPhysicalDeviceProperties2") == 0 ||
        strcmp(name, "vkGetPhysicalDeviceFormatProperties2") == 0 ||
        strcmp(name, "vkGetPhysicalDeviceImageFormatProperties2") == 0 ||
        strcmp(name, "vkGetPhysicalDeviceQueueFamilyProperties2") == 0 ||
        strcmp(name, "vkGetPhysicalDeviceMemoryProperties2") == 0 ||
        strcmp(name, "vkGetPhysicalDeviceSparseImageFormatProperties2") == 0 ||
        strcmp(name, "vkGetPhysicalDeviceExternalBufferProperties") == 0 ||
        strcmp(name, "vkGetPhysicalDeviceExternalFenceProperties") == 0 ||
        strcmp(name, "vkGetPhysicalDeviceExternalSemaphoreProperties") == 0)
        return reinterpret_cast<PFN_vkVoidFunction>(NoOp);

    // Per the spec, return NULL for nonexistent entrypoints.
    return nullptr;
}

} // namespace vkstubhal

namespace {

#ifdef VK_USE_PLATFORM_ANDROID_KHR

int OpenDevice(const hw_module_t* module, const char* id, hw_device_t** device);

hw_module_methods_t goldfish_vulkan_module_methods = {
    .open = OpenDevice
};

extern "C" __attribute__((visibility("default"))) hwvulkan_module_t HAL_MODULE_INFO_SYM = {
    .common = {
        .tag = HARDWARE_MODULE_TAG,
        .module_api_version = HWVULKAN_MODULE_API_VERSION_0_1,
        .hal_api_version = HARDWARE_HAL_API_VERSION,
        .id = HWVULKAN_HARDWARE_MODULE_ID,
        .name = "Goldfish Vulkan Driver",
        .author = "The Android Open Source Project",
        .methods = &goldfish_vulkan_module_methods,
    },
};

int CloseDevice(struct hw_device_t* /*device*/) {
    AEMU_SCOPED_TRACE("goldfish_vulkan::GetInstanceProcAddr");
    // nothing to do - opening a device doesn't allocate any resources
    return 0;
}

#endif

#define VK_HOST_CONNECTION(ret) \
    HostConnection *hostCon = HostConnection::getOrCreate(VIRTIO_GPU_CAPSET_GFXSTREAM); \
    if (!hostCon) { \
        ALOGE("vulkan: Failed to get host connection\n"); \
        return ret; \
    } \
    ExtendedRCEncoderContext *rcEnc = hostCon->rcEncoder(); \
    if (!rcEnc) { \
        ALOGE("vulkan: Failed to get renderControl encoder context\n"); \
        return ret; \
    } \
    gfxstream::vk::ResourceTracker::ThreadingCallbacks threadingCallbacks = { \
        [] { \
          auto hostCon = HostConnection::get(); \
          hostCon->rcEncoder(); \
          return hostCon; \
        }, \
        [](HostConnection* hostCon) { return hostCon->vkEncoder(); }, \
    }; \
    gfxstream::vk::ResourceTracker::get()->setThreadingCallbacks(threadingCallbacks); \
    gfxstream::vk::ResourceTracker::get()->setupFeatures(rcEnc->featureInfo_const()); \
    gfxstream::vk::ResourceTracker::get()->setupCaps();                                    \
    gfxstream::vk::ResourceTracker::get()->setSeqnoPtr(getSeqnoPtrForProcess()); \
    auto hostSupportsVulkan = gfxstream::vk::ResourceTracker::get()->hostSupportsVulkan(); \
    gfxstream::vk::VkEncoder *vkEnc = hostCon->vkEncoder(); \
    if (!vkEnc) { \
        ALOGE("vulkan: Failed to get Vulkan encoder\n"); \
        return ret; \
    } \

VKAPI_ATTR
VkResult EnumerateInstanceExtensionProperties(
    const char* layer_name,
    uint32_t* count,
    VkExtensionProperties* properties) {
    AEMU_SCOPED_TRACE("goldfish_vulkan::EnumerateInstanceExtensionProperties");

    VK_HOST_CONNECTION(VK_ERROR_DEVICE_LOST)

    if (!hostSupportsVulkan) {
        return vkstubhal::EnumerateInstanceExtensionProperties(layer_name, count, properties);
    }

    if (layer_name) {
        ALOGW(
            "Driver vkEnumerateInstanceExtensionProperties shouldn't be called "
            "with a layer name ('%s')",
            layer_name);
    }

    VkResult res = gfxstream::vk::ResourceTracker::get()->on_vkEnumerateInstanceExtensionProperties(
        vkEnc, VK_SUCCESS, layer_name, count, properties);

    return res;
}

VKAPI_ATTR
VkResult CreateInstance(const VkInstanceCreateInfo* create_info,
                        const VkAllocationCallbacks* allocator,
                        VkInstance* out_instance) {
    AEMU_SCOPED_TRACE("goldfish_vulkan::CreateInstance");

    VK_HOST_CONNECTION(VK_ERROR_DEVICE_LOST)

    if (!hostSupportsVulkan) {
        return vkstubhal::CreateInstance(create_info, allocator, out_instance);
    }

    VkResult res = vkEnc->vkCreateInstance(create_info, nullptr, out_instance, true /* do lock */);

    return res;
}

#ifdef VK_USE_PLATFORM_FUCHSIA
VKAPI_ATTR
VkResult GetMemoryZirconHandleFUCHSIA(
    VkDevice device,
    const VkMemoryGetZirconHandleInfoFUCHSIA* pInfo,
    uint32_t* pHandle) {
    AEMU_SCOPED_TRACE("goldfish_vulkan::GetMemoryZirconHandleFUCHSIA");

    VK_HOST_CONNECTION(VK_ERROR_DEVICE_LOST)

    if (!hostSupportsVulkan) {
        return vkstubhal::GetMemoryZirconHandleFUCHSIA(device, pInfo, pHandle);
    }

    VkResult res = gfxstream::vk::ResourceTracker::get()->
        on_vkGetMemoryZirconHandleFUCHSIA(
            vkEnc, VK_SUCCESS,
            device, pInfo, pHandle);

    return res;
}

VKAPI_ATTR
VkResult GetMemoryZirconHandlePropertiesFUCHSIA(
    VkDevice device,
    VkExternalMemoryHandleTypeFlagBits handleType,
    uint32_t handle,
    VkMemoryZirconHandlePropertiesFUCHSIA* pProperties) {
    AEMU_SCOPED_TRACE("goldfish_vulkan::GetMemoryZirconHandlePropertiesFUCHSIA");

    VK_HOST_CONNECTION(VK_ERROR_DEVICE_LOST)

    if (!hostSupportsVulkan) {
        return vkstubhal::GetMemoryZirconHandlePropertiesFUCHSIA(
            device, handleType, handle, pProperties);
    }

    VkResult res = gfxstream::vk::ResourceTracker::get()->
        on_vkGetMemoryZirconHandlePropertiesFUCHSIA(
            vkEnc, VK_SUCCESS, device, handleType, handle, pProperties);

    return res;
}

VKAPI_ATTR
VkResult GetSemaphoreZirconHandleFUCHSIA(
    VkDevice device,
    const VkSemaphoreGetZirconHandleInfoFUCHSIA* pInfo,
    uint32_t* pHandle) {
    AEMU_SCOPED_TRACE("goldfish_vulkan::GetSemaphoreZirconHandleFUCHSIA");

    VK_HOST_CONNECTION(VK_ERROR_DEVICE_LOST)

    if (!hostSupportsVulkan) {
        return vkstubhal::GetSemaphoreZirconHandleFUCHSIA(device, pInfo, pHandle);
    }

    VkResult res = gfxstream::vk::ResourceTracker::get()->
        on_vkGetSemaphoreZirconHandleFUCHSIA(
            vkEnc, VK_SUCCESS, device, pInfo, pHandle);

    return res;
}

VKAPI_ATTR
VkResult ImportSemaphoreZirconHandleFUCHSIA(
    VkDevice device,
    const VkImportSemaphoreZirconHandleInfoFUCHSIA* pInfo) {
    AEMU_SCOPED_TRACE("goldfish_vulkan::ImportSemaphoreZirconHandleFUCHSIA");

    VK_HOST_CONNECTION(VK_ERROR_DEVICE_LOST)

    if (!hostSupportsVulkan) {
        return vkstubhal::ImportSemaphoreZirconHandleFUCHSIA(device, pInfo);
    }

    VkResult res = gfxstream::vk::ResourceTracker::get()->
        on_vkImportSemaphoreZirconHandleFUCHSIA(
            vkEnc, VK_SUCCESS, device, pInfo);

    return res;
}

VKAPI_ATTR
VkResult CreateBufferCollectionFUCHSIA(
    VkDevice device,
    const VkBufferCollectionCreateInfoFUCHSIA* pInfo,
    const VkAllocationCallbacks* pAllocator,
    VkBufferCollectionFUCHSIA* pCollection) {
    AEMU_SCOPED_TRACE("goldfish_vulkan::CreateBufferCollectionFUCHSIA");

    VK_HOST_CONNECTION(VK_ERROR_DEVICE_LOST)

    if (!hostSupportsVulkan) {
        return vkstubhal::CreateBufferCollectionFUCHSIA(
            device, pInfo, pAllocator, pCollection);
    }

    VkResult res =
        gfxstream::vk::ResourceTracker::get()->on_vkCreateBufferCollectionFUCHSIA(
            vkEnc, VK_SUCCESS, device, pInfo, pAllocator, pCollection);

    return res;
}

VKAPI_ATTR
void DestroyBufferCollectionFUCHSIA(VkDevice device,
                                    VkBufferCollectionFUCHSIA collection,
                                    const VkAllocationCallbacks* pAllocator) {
    AEMU_SCOPED_TRACE("goldfish_vulkan::DestroyBufferCollectionFUCHSIA");

    VK_HOST_CONNECTION()

    if (!hostSupportsVulkan) {
        vkstubhal::DestroyBufferCollectionFUCHSIA(device, collection,
                                                  pAllocator);
        return;
    }

    gfxstream::vk::ResourceTracker::get()->on_vkDestroyBufferCollectionFUCHSIA(
        vkEnc, VK_SUCCESS, device, collection, pAllocator);
}

VKAPI_ATTR
VkResult SetBufferCollectionBufferConstraintsFUCHSIA(
    VkDevice device,
    VkBufferCollectionFUCHSIA collection,
    const VkBufferConstraintsInfoFUCHSIA* pBufferConstraintsInfo) {
    AEMU_SCOPED_TRACE(
        "goldfish_vulkan::SetBufferCollectionBufferConstraintsFUCHSIA");

    VK_HOST_CONNECTION(VK_ERROR_DEVICE_LOST)

    if (!hostSupportsVulkan) {
        return vkstubhal::SetBufferCollectionBufferConstraintsFUCHSIA(
            device, collection, pBufferConstraintsInfo);
    }

    VkResult res =
        gfxstream::vk::ResourceTracker::get()
            ->on_vkSetBufferCollectionBufferConstraintsFUCHSIA(
                vkEnc, VK_SUCCESS, device, collection, pBufferConstraintsInfo);

    return res;
}

VKAPI_ATTR
VkResult SetBufferCollectionImageConstraintsFUCHSIA(
    VkDevice device,
    VkBufferCollectionFUCHSIA collection,
    const VkImageConstraintsInfoFUCHSIA* pImageConstraintsInfo) {
    AEMU_SCOPED_TRACE(
        "goldfish_vulkan::SetBufferCollectionBufferConstraintsFUCHSIA");

    VK_HOST_CONNECTION(VK_ERROR_DEVICE_LOST)

    if (!hostSupportsVulkan) {
        return vkstubhal::SetBufferCollectionImageConstraintsFUCHSIA(
            device, collection, pImageConstraintsInfo);
    }

    VkResult res =
        gfxstream::vk::ResourceTracker::get()
            ->on_vkSetBufferCollectionImageConstraintsFUCHSIA(
                vkEnc, VK_SUCCESS, device, collection, pImageConstraintsInfo);

    return res;
}

VKAPI_ATTR
VkResult GetBufferCollectionPropertiesFUCHSIA(
    VkDevice device,
    VkBufferCollectionFUCHSIA collection,
    VkBufferCollectionPropertiesFUCHSIA* pProperties) {
    AEMU_SCOPED_TRACE("goldfish_vulkan::GetBufferCollectionPropertiesFUCHSIA");

    VK_HOST_CONNECTION(VK_ERROR_DEVICE_LOST)

    if (!hostSupportsVulkan) {
        return vkstubhal::GetBufferCollectionPropertiesFUCHSIA(
            device, collection, pProperties);
    }

    VkResult res = gfxstream::vk::ResourceTracker::get()
                       ->on_vkGetBufferCollectionPropertiesFUCHSIA(
                           vkEnc, VK_SUCCESS, device, collection, pProperties);

    return res;
}

#endif

uint64_t currGuestTimeNs() {
    struct timespec ts;
#ifdef __APPLE__
    clock_gettime(CLOCK_REALTIME, &ts);
#else
    clock_gettime(CLOCK_BOOTTIME, &ts);
#endif
    uint64_t res = (uint64_t)(ts.tv_sec * 1000000000ULL + ts.tv_nsec);
    return res;
}

struct FrameTracingState {
    uint32_t frameNumber = 0;
    bool tracingEnabled = false;
    void onSwapBuffersSuccessful(ExtendedRCEncoderContext* rcEnc) {
#ifdef GFXSTREAM
        bool current = android::base::isTracingEnabled();
        // edge trigger
        if (current && !tracingEnabled) {
            if (rcEnc->hasHostSideTracing()) {
                rcEnc->rcSetTracingForPuid(rcEnc, getPuid(), 1, currGuestTimeNs());
            }
        }
        if (!current && tracingEnabled) {
            if (rcEnc->hasHostSideTracing()) {
                rcEnc->rcSetTracingForPuid(rcEnc, getPuid(), 0, currGuestTimeNs());
            }
        }
        tracingEnabled = current;
#endif
        ++frameNumber;
    }
};

static FrameTracingState sFrameTracingState;

static PFN_vkVoidFunction sQueueSignalReleaseImageAndroidImpl = 0;

#ifdef VK_USE_PLATFORM_ANDROID_KHR
static VkResult
QueueSignalReleaseImageANDROID(
    VkQueue queue,
    uint32_t waitSemaphoreCount,
    const VkSemaphore* pWaitSemaphores,
    VkImage image,
    int* pNativeFenceFd)
{
    sFrameTracingState.onSwapBuffersSuccessful(HostConnection::get()->rcEncoder());
    ((PFN_vkQueueSignalReleaseImageANDROID)sQueueSignalReleaseImageAndroidImpl)(queue, waitSemaphoreCount, pWaitSemaphores, image, pNativeFenceFd);
    return VK_SUCCESS;
}
#endif

static PFN_vkVoidFunction GetDeviceProcAddr(VkDevice device, const char* name) {
    AEMU_SCOPED_TRACE("goldfish_vulkan::GetDeviceProcAddr");

    VK_HOST_CONNECTION(nullptr)

    if (!hostSupportsVulkan) {
        return nullptr;
    }

#ifdef VK_USE_PLATFORM_FUCHSIA
    if (!strcmp(name, "vkGetMemoryZirconHandleFUCHSIA")) {
        return (PFN_vkVoidFunction)GetMemoryZirconHandleFUCHSIA;
    }
    if (!strcmp(name, "vkGetMemoryZirconHandlePropertiesFUCHSIA")) {
        return (PFN_vkVoidFunction)GetMemoryZirconHandlePropertiesFUCHSIA;
    }
    if (!strcmp(name, "vkGetSemaphoreZirconHandleFUCHSIA")) {
        return (PFN_vkVoidFunction)GetSemaphoreZirconHandleFUCHSIA;
    }
    if (!strcmp(name, "vkImportSemaphoreZirconHandleFUCHSIA")) {
        return (PFN_vkVoidFunction)ImportSemaphoreZirconHandleFUCHSIA;
    }
    if (!strcmp(name, "vkCreateBufferCollectionFUCHSIA")) {
        return (PFN_vkVoidFunction)CreateBufferCollectionFUCHSIA;
    }
    if (!strcmp(name, "vkDestroyBufferCollectionFUCHSIA")) {
        return (PFN_vkVoidFunction)DestroyBufferCollectionFUCHSIA;
    }
    if (!strcmp(name, "vkSetBufferCollectionImageConstraintsFUCHSIA")) {
        return (PFN_vkVoidFunction)SetBufferCollectionImageConstraintsFUCHSIA;
    }
    if (!strcmp(name, "vkSetBufferCollectionBufferConstraintsFUCHSIA")) {
        return (PFN_vkVoidFunction)SetBufferCollectionBufferConstraintsFUCHSIA;
    }
    if (!strcmp(name, "vkGetBufferCollectionPropertiesFUCHSIA")) {
        return (PFN_vkVoidFunction)GetBufferCollectionPropertiesFUCHSIA;
    }
#endif
#ifdef VK_USE_PLATFORM_ANDROID_KHR
    if (!strcmp(name, "vkQueueSignalReleaseImageANDROID")) {
        if (!sQueueSignalReleaseImageAndroidImpl) {
            sQueueSignalReleaseImageAndroidImpl =
                (PFN_vkVoidFunction)(
                    gfxstream::vk::goldfish_vulkan_get_device_proc_address(device, "vkQueueSignalReleaseImageANDROID"));
        }
        return (PFN_vkVoidFunction)QueueSignalReleaseImageANDROID;
    }
#endif
    if (!strcmp(name, "vkGetDeviceProcAddr")) {
        return (PFN_vkVoidFunction)(GetDeviceProcAddr);
    }
    return (PFN_vkVoidFunction)(gfxstream::vk::goldfish_vulkan_get_device_proc_address(device, name));
}

VKAPI_ATTR
PFN_vkVoidFunction GetInstanceProcAddr(VkInstance instance, const char* name) {
    AEMU_SCOPED_TRACE("goldfish_vulkan::GetInstanceProcAddr");

    VK_HOST_CONNECTION(nullptr)

    if (!hostSupportsVulkan) {
        return vkstubhal::GetInstanceProcAddr(instance, name);
    }

    if (!strcmp(name, "vkEnumerateInstanceExtensionProperties")) {
        return (PFN_vkVoidFunction)EnumerateInstanceExtensionProperties;
    }
    if (!strcmp(name, "vkCreateInstance")) {
        return (PFN_vkVoidFunction)CreateInstance;
    }
    if (!strcmp(name, "vkGetDeviceProcAddr")) {
        return (PFN_vkVoidFunction)(GetDeviceProcAddr);
    }
#ifdef VK_USE_PLATFORM_ANDROID_KHR
    if (!strcmp(name, "vkQueueSignalReleaseImageANDROID")) {
        if (!sQueueSignalReleaseImageAndroidImpl) {
            sQueueSignalReleaseImageAndroidImpl =
                (PFN_vkVoidFunction)(
                    gfxstream::vk::goldfish_vulkan_get_instance_proc_address(instance, "vkQueueSignalReleaseImageANDROID"));
        }
        return (PFN_vkVoidFunction)QueueSignalReleaseImageANDROID;
    }
#endif
    return (PFN_vkVoidFunction)(gfxstream::vk::goldfish_vulkan_get_instance_proc_address(instance, name));
}

#ifdef VK_USE_PLATFORM_ANDROID_KHR

hwvulkan_device_t goldfish_vulkan_device = {
    .common = {
        .tag = HARDWARE_DEVICE_TAG,
        .version = HWVULKAN_DEVICE_API_VERSION_0_1,
        .module = &HAL_MODULE_INFO_SYM.common,
        .close = CloseDevice,
    },
    .EnumerateInstanceExtensionProperties = EnumerateInstanceExtensionProperties,
    .CreateInstance = CreateInstance,
    .GetInstanceProcAddr = GetInstanceProcAddr,
};

int OpenDevice(const hw_module_t* /*module*/,
               const char* id,
               hw_device_t** device) {
    AEMU_SCOPED_TRACE("goldfish_vulkan::OpenDevice");

    if (strcmp(id, HWVULKAN_DEVICE_0) == 0) {
        *device = &goldfish_vulkan_device.common;
        gfxstream::vk::ResourceTracker::get();
        return 0;
    }
    return -ENOENT;
}

#elif VK_USE_PLATFORM_FUCHSIA

class VulkanDevice {
public:
    VulkanDevice() : mHostSupportsGoldfish(IsAccessible(QEMU_PIPE_PATH)) {
        InitLogger();
        InitTraceProvider();
        gfxstream::vk::ResourceTracker::get();
    }

    static void InitLogger();

    static bool IsAccessible(const char* name) {
        zx_handle_t handle = GetConnectToServiceFunction()(name);
        if (handle == ZX_HANDLE_INVALID)
            return false;

        zxio_storage_t io_storage;
        zx_status_t status = zxio_create(handle, &io_storage);
        if (status != ZX_OK)
            return false;

        status = zxio_close(&io_storage.io, /*should_wait=*/true);
        if (status != ZX_OK)
            return false;

        return true;
    }

    static VulkanDevice& GetInstance() {
        static VulkanDevice g_instance;
        return g_instance;
    }

    PFN_vkVoidFunction GetInstanceProcAddr(VkInstance instance, const char* name) {
        if (!mHostSupportsGoldfish) {
            return vkstubhal::GetInstanceProcAddr(instance, name);
        }
        return ::GetInstanceProcAddr(instance, name);
    }

private:
    void InitTraceProvider();

    TraceProviderFuchsia mTraceProvider;
    const bool mHostSupportsGoldfish;
};

void VulkanDevice::InitLogger() {
  auto log_socket = ([] () -> std::optional<zx::socket> {
    fidl::ClientEnd<fuchsia_logger::LogSink> channel{zx::channel{
      GetConnectToServiceFunction()("/svc/fuchsia.logger.LogSink")}};
    if (!channel.is_valid())
      return std::nullopt;

    zx::socket local_socket, remote_socket;
    zx_status_t status = zx::socket::create(ZX_SOCKET_DATAGRAM, &local_socket, &remote_socket);
    if (status != ZX_OK)
      return std::nullopt;

    auto result = fidl::WireCall(channel)->Connect(std::move(remote_socket));

    if (!result.ok())
      return std::nullopt;

    return local_socket;
  })();
  if (!log_socket)
    return;

  fx_logger_config_t config = {
      .min_severity = FX_LOG_INFO,
      .log_sink_socket = log_socket->release(),
      .tags = nullptr,
      .num_tags = 0,
  };

  fx_log_reconfigure(&config);
}

void VulkanDevice::InitTraceProvider() {
    if (!mTraceProvider.Initialize()) {
        ALOGE("Trace provider failed to initialize");
    }
}

extern "C" __attribute__((visibility("default"))) PFN_vkVoidFunction
vk_icdGetInstanceProcAddr(VkInstance instance, const char* name) {
    return VulkanDevice::GetInstance().GetInstanceProcAddr(instance, name);
}

extern "C" __attribute__((visibility("default"))) VkResult
vk_icdNegotiateLoaderICDInterfaceVersion(uint32_t* pSupportedVersion) {
    *pSupportedVersion = std::min(*pSupportedVersion, 3u);
    return VK_SUCCESS;
}

typedef VkResult(VKAPI_PTR *PFN_vkOpenInNamespaceAddr)(const char *pName, uint32_t handle);

namespace {

PFN_vkOpenInNamespaceAddr g_vulkan_connector;

zx_handle_t LocalConnectToServiceFunction(const char* pName) {
    zx::channel remote_endpoint, local_endpoint;
    zx_status_t status;
    if ((status = zx::channel::create(0, &remote_endpoint, &local_endpoint)) != ZX_OK) {
        ALOGE("zx::channel::create failed: %d", status);
        return ZX_HANDLE_INVALID;
    }
    if ((status = g_vulkan_connector(pName, remote_endpoint.release())) != ZX_OK) {
        ALOGE("vulkan_connector failed: %d", status);
        return ZX_HANDLE_INVALID;
    }
    return local_endpoint.release();
}

}

extern "C" __attribute__((visibility("default"))) void
vk_icdInitializeOpenInNamespaceCallback(PFN_vkOpenInNamespaceAddr callback) {
    g_vulkan_connector = callback;
    SetConnectToServiceFunction(&LocalConnectToServiceFunction);
}

#else
class VulkanDevice {
public:
    VulkanDevice() {
        gfxstream::vk::ResourceTracker::get();
    }

    static VulkanDevice& GetInstance() {
        static VulkanDevice g_instance;
        return g_instance;
    }

    PFN_vkVoidFunction GetInstanceProcAddr(VkInstance instance, const char* name) {
        return ::GetInstanceProcAddr(instance, name);
    }
};

extern "C" __attribute__((visibility("default"))) PFN_vkVoidFunction
vk_icdGetInstanceProcAddr(VkInstance instance, const char* name) {
    return VulkanDevice::GetInstance().GetInstanceProcAddr(instance, name);
}

extern "C" __attribute__((visibility("default"))) VkResult
vk_icdNegotiateLoaderICDInterfaceVersion(uint32_t* pSupportedVersion) {
    *pSupportedVersion = std::min(*pSupportedVersion, 3u);
    return VK_SUCCESS;
}

#endif

} // namespace
