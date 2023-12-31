// Copyright 2017 The Android Open Source Project
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

#include "host-common/opengl/macTouchOpenGL.h"

#include <Cocoa/Cocoa.h>
#include <OpenGL/OpenGL.h>

static NSOpenGLPixelFormatAttribute testAttrs[] =
{
    NSOpenGLPFADoubleBuffer,
    NSOpenGLPFAWindow,
    NSOpenGLPFAPixelBuffer,
    NSOpenGLPFAColorSize   ,32,
    NSOpenGLPFADepthSize   ,24,
    NSOpenGLPFAStencilSize ,8,
    0
};

// In order for IOKit to know the latest GPU selection status, we need
// to touch OpenGL a little bit. Here, we allocate and initialize a pixel
// format, which seems to be enough.
void macTouchOpenGL() {
    void* res = [[NSOpenGLPixelFormat alloc] initWithAttributes:testAttrs];
    [res release];
}
