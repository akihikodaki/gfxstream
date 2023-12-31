/*
 * Copyright © 2016 Intel Corporation
 * Copyright © 2019 The Android Open Source Project
 * Copyright © 2019 Google Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef VK_FORMAT_INFO_H
#define VK_FORMAT_INFO_H

#include <stdbool.h>
#ifdef VK_USE_PLATFORM_ANDROID_KHR
#include <system/graphics.h>
#else
/* See system/graphics.h. */
enum {
    HAL_PIXEL_FORMAT_YV12 = 842094169,
};
#endif
#include <vulkan/vulkan.h>
#include <vndk/hardware_buffer.h>

namespace gfxstream {
namespace vk {

/* See i915_private_android_types.h in minigbm. */
#define HAL_PIXEL_FORMAT_NV12_Y_TILED_INTEL 0x100

// TODO(b/167698976): We should not use OMX_COLOR_FormatYUV420Planar
// but we seem to miss a format translation somewhere.

#define OMX_COLOR_FormatYUV420Planar 0x13

// TODO: update users of this function to query the DRM fourcc
// code using the standard Gralloc4 metadata type and instead
// translate the DRM fourcc code to a Vulkan format as Android
// formats such as AHARDWAREBUFFER_FORMAT_Y8Cb8Cr8_420 could be
// either VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM or
// VK_FORMAT_G8_B8R8_2PLANE_420_UNORM.
static inline VkFormat
vk_format_from_android(unsigned android_format)
{
   switch (android_format) {
   case AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM:
      return VK_FORMAT_R8G8B8A8_UNORM;
   case AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM:
      return VK_FORMAT_R8G8B8A8_UNORM;
   case AHARDWAREBUFFER_FORMAT_R8G8B8_UNORM:
      return VK_FORMAT_R8G8B8_UNORM;
   case AHARDWAREBUFFER_FORMAT_R5G6B5_UNORM:
      return VK_FORMAT_R5G6B5_UNORM_PACK16;
   case AHARDWAREBUFFER_FORMAT_R16G16B16A16_FLOAT:
      return VK_FORMAT_R16G16B16A16_SFLOAT;
   case AHARDWAREBUFFER_FORMAT_R10G10B10A2_UNORM:
      return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
   case HAL_PIXEL_FORMAT_NV12_Y_TILED_INTEL:
   case AHARDWAREBUFFER_FORMAT_Y8Cb8Cr8_420:
      return VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
#if __ANDROID_API__ >= 30
   case AHARDWAREBUFFER_FORMAT_YCbCr_P010:
      return VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16;
#endif
#ifdef VK_USE_PLATFORM_ANDROID_KHR
   case HAL_PIXEL_FORMAT_YV12:
   case OMX_COLOR_FormatYUV420Planar:
      return VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM;
   case AHARDWAREBUFFER_FORMAT_BLOB:
#endif
   default:
      return VK_FORMAT_UNDEFINED;
   }
}

static inline unsigned
android_format_from_vk(VkFormat vk_format)
{
   switch (vk_format) {
   case VK_FORMAT_R8G8B8A8_UNORM:
      return AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM;
   case VK_FORMAT_R8G8B8_UNORM:
      return AHARDWAREBUFFER_FORMAT_R8G8B8_UNORM;
   case VK_FORMAT_R5G6B5_UNORM_PACK16:
      return AHARDWAREBUFFER_FORMAT_R5G6B5_UNORM;
   case VK_FORMAT_R16G16B16A16_SFLOAT:
      return AHARDWAREBUFFER_FORMAT_R16G16B16A16_FLOAT;
   case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
      return AHARDWAREBUFFER_FORMAT_R10G10B10A2_UNORM;
   case VK_FORMAT_G8_B8R8_2PLANE_420_UNORM:
      return HAL_PIXEL_FORMAT_NV12_Y_TILED_INTEL;
   case VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM:
      return HAL_PIXEL_FORMAT_YV12;
   default:
      return AHARDWAREBUFFER_FORMAT_BLOB;
   }
}

static inline bool
android_format_is_yuv(unsigned android_format)
{
   switch (android_format) {
   case AHARDWAREBUFFER_FORMAT_BLOB:
   case AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM:
   case AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM:
   case AHARDWAREBUFFER_FORMAT_R8G8B8_UNORM:
   case AHARDWAREBUFFER_FORMAT_R5G6B5_UNORM:
   case AHARDWAREBUFFER_FORMAT_R16G16B16A16_FLOAT:
   case AHARDWAREBUFFER_FORMAT_R10G10B10A2_UNORM:
   case AHARDWAREBUFFER_FORMAT_D16_UNORM:
   case AHARDWAREBUFFER_FORMAT_D24_UNORM:
   case AHARDWAREBUFFER_FORMAT_D24_UNORM_S8_UINT:
   case AHARDWAREBUFFER_FORMAT_D32_FLOAT:
   case AHARDWAREBUFFER_FORMAT_D32_FLOAT_S8_UINT:
   case AHARDWAREBUFFER_FORMAT_S8_UINT:
      return false;
   case HAL_PIXEL_FORMAT_NV12_Y_TILED_INTEL:
   case OMX_COLOR_FormatYUV420Planar:
   case HAL_PIXEL_FORMAT_YV12:
#if __ANDROID_API__ >= 30
   case AHARDWAREBUFFER_FORMAT_YCbCr_P010:
#endif
   case AHARDWAREBUFFER_FORMAT_Y8Cb8Cr8_420:
      return true;
   default:
      ALOGE("%s: unhandled format: %d", __FUNCTION__, android_format);
      return false;
   }
}

static inline VkImageAspectFlags
vk_format_aspects(VkFormat format)
{
   switch (format) {
   case VK_FORMAT_UNDEFINED:
      return 0;

   case VK_FORMAT_S8_UINT:
      return VK_IMAGE_ASPECT_STENCIL_BIT;

   case VK_FORMAT_D16_UNORM_S8_UINT:
   case VK_FORMAT_D24_UNORM_S8_UINT:
   case VK_FORMAT_D32_SFLOAT_S8_UINT:
      return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;

   case VK_FORMAT_D16_UNORM:
   case VK_FORMAT_X8_D24_UNORM_PACK32:
   case VK_FORMAT_D32_SFLOAT:
      return VK_IMAGE_ASPECT_DEPTH_BIT;

   case VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM:
   case VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM:
   case VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM:
   case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16:
   case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16:
   case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16:
   case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16:
   case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16:
   case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16:
   case VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM:
   case VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM:
   case VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM:
      return (VK_IMAGE_ASPECT_PLANE_0_BIT |
              VK_IMAGE_ASPECT_PLANE_1_BIT |
              VK_IMAGE_ASPECT_PLANE_2_BIT);

   case VK_FORMAT_G8_B8R8_2PLANE_420_UNORM:
   case VK_FORMAT_G8_B8R8_2PLANE_422_UNORM:
   case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16:
   case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16:
   case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16:
   case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16:
   case VK_FORMAT_G16_B16R16_2PLANE_420_UNORM:
   case VK_FORMAT_G16_B16R16_2PLANE_422_UNORM:
      return (VK_IMAGE_ASPECT_PLANE_0_BIT |
              VK_IMAGE_ASPECT_PLANE_1_BIT);

   default:
      return VK_IMAGE_ASPECT_COLOR_BIT;
   }
}

static inline bool
vk_format_is_color(VkFormat format)
{
   return vk_format_aspects(format) == VK_IMAGE_ASPECT_COLOR_BIT;
}

static inline bool
vk_format_is_depth_or_stencil(VkFormat format)
{
   const VkImageAspectFlags aspects = vk_format_aspects(format);
   return aspects & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
}

static inline bool
vk_format_has_depth(VkFormat format)
{
   const VkImageAspectFlags aspects = vk_format_aspects(format);
   return aspects & VK_IMAGE_ASPECT_DEPTH_BIT;
}

}  // namespace vk
}  // namespace gfxstream

#endif /* VK_FORMAT_INFO_H */
