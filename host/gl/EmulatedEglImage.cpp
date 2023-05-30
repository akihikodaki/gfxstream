/*
* Copyright (C) 2022 The Android Open Source Project
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include "EmulatedEglImage.h"

#include "OpenGLESDispatch/DispatchTables.h"
#include "OpenGLESDispatch/EGLDispatch.h"
#include "host-common/logging.h"

namespace gfxstream {
namespace gl {

/*static*/
std::unique_ptr<EmulatedEglImage> EmulatedEglImage::create(EGLDisplay display,
                                                           EGLContext context,
                                                           EGLenum target,
                                                           EGLClientBuffer buffer) {
    EGLImageKHR image = s_egl.eglCreateImageKHR(display, context, target, buffer, nullptr);
    if (image == EGL_NO_IMAGE_KHR) {
        ERR("Failed to create EGL image.");
        return nullptr;
    }

    // Note: `handle` is (historically) the underlying image handle potentially so that
    // it can be used directly by glEGLImageTargetTexture2DOES() without another lookup.
    // In the future, EmulatedEglImage could be updated to use a handle generated by
    // FrameBuffer.
    HandleType handle = (HandleType)reinterpret_cast<uintptr_t>(image);

    return std::unique_ptr<EmulatedEglImage>(new EmulatedEglImage(handle, display, image));
}

EmulatedEglImage::EmulatedEglImage(HandleType handle,
                                   EGLDisplay display,
                                   EGLImageKHR image)
    : mHandle(handle),
      mEglDisplay(display),
      mEglImage(image) {}

EmulatedEglImage::~EmulatedEglImage() {
    destroy();
}

EGLBoolean EmulatedEglImage::destroy() {
    if (mEglImage) {
        EGLBoolean ret = s_egl.eglDestroyImageKHR(mEglDisplay, mEglImage);
        if (!ret) {
            ERR("Failed to destroy EGL image.");
        }
        mEglImage = EGL_NO_IMAGE_KHR;
        return ret;
    }
    return EGL_TRUE;
}

}  // namespace gl
}  // namespace gfxstream