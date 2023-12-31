/*
* Copyright (C) 2011 The Android Open Source Project
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
#include <Cocoa/Cocoa.h>
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl3.h>
#include <stdio.h>
#include "MacPixelFormatsAttribs.h"

//
// EmuGLContext inherit from NSOpenGLContext
// and adds binding state for the context to know
// if it was last bounded to a pbuffer or a window.
// This is because after the context was bounded to
// a Pbuffer, before we bind it to a window we must
// release it form the pbuffer by calling the
// clearDrawable method. We do not want to call clearDrawable
// more than really needed since when it is called at a time
// that a window is bounded to the context it will clear the
// window content causing flickering effect.
// Thererfore we call clearDrawable only when we bind the context
// to a window and it was previously bound to a Pbuffer.
//
@interface EmuGLContext : NSOpenGLContext {
    @private
        int boundToPbuffer;
        int boundToWin;
    @public
        GLuint ytexForDecoder;
        GLuint uvtexForDecoder;
}

- (id) initWithFormat:(NSOpenGLPixelFormat *)pixelFormat shareContext:(NSOpenGLContext *)share;
- (void) preBind:(int)forPbuffer;
@end

@implementation EmuGLContext
- (id) initWithFormat:(NSOpenGLPixelFormat *)pixelFormat shareContext:(NSOpenGLContext *)share
{
    self = [super initWithFormat:pixelFormat shareContext:share];
    if (self != nil) {
        boundToPbuffer = 0;
        boundToWin = 0;
        ytexForDecoder = 0;
        uvtexForDecoder = 0;
    }
    return self;
}

- (void) preBind:(int)forPbuffer
{
    if ((!forPbuffer && boundToPbuffer)) {
        [self clearDrawable]; 
    }
    boundToPbuffer = forPbuffer;
    boundToWin = !boundToPbuffer;
}
@end

int getAttrListLength(const NSOpenGLPixelFormatAttribute* list) {
    int count = 0;
    while (list[count++] != 0);
    return count ? (count - 1) : 0;
}

static const NSOpenGLPixelFormatAttribute core32TestProfile[] = {
    NSOpenGLPFAAccelerated,
    NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core,
    NSOpenGLPFADoubleBuffer,
    NSOpenGLPFAColorSize   ,32,
    NSOpenGLPFADepthSize   ,24,
    NSOpenGLPFAStencilSize ,8,
    0
};

static const NSOpenGLPixelFormatAttribute core41TestProfile[] = {
    NSOpenGLPFAAccelerated,
    NSOpenGLPFANoRecovery,
    NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion4_1Core,
    NSOpenGLPFADoubleBuffer,
    NSOpenGLPFAColorSize   ,32,
    NSOpenGLPFADepthSize   ,24,
    NSOpenGLPFAStencilSize ,8,
    0
};

int setupCoreProfileNativeFormats() {

    NSOpenGLPixelFormat* core41Supported =
        [[NSOpenGLPixelFormat alloc] initWithAttributes: core41TestProfile];

    if (core41Supported) {
        setCoreProfileLevel(NSOpenGLProfileVersion4_1Core);
        [core41Supported release];
        return (int)NSOpenGLProfileVersion4_1Core;
    }

    NSOpenGLPixelFormat* core32Supported =
        [[NSOpenGLPixelFormat alloc] initWithAttributes: core32TestProfile];

    if (core32Supported) {
        setCoreProfileLevel(NSOpenGLProfileVersion3_2Core);
        [core32Supported release];
        return (int)NSOpenGLProfileVersion3_2Core;
    }

    return (int)NSOpenGLProfileVersionLegacy;
}

int getNumPixelFormats(){
    int size;
    const NSOpenGLPixelFormatAttribute* const* attrib_lists =
        getPixelFormatsAttributes(&size);
    return size;
}

void* finalizePixelFormat(bool coreProfile,
                          int attribsId) {
    int size;
    const NSOpenGLPixelFormatAttribute* const* attrib_lists =
        getPixelFormatsAttributes(&size);

    assert(attribsId < size);

    const NSOpenGLPixelFormatAttribute*  attrs =
        attrib_lists[attribsId];

    const NSOpenGLPixelFormatAttribute* selected_variant =
        coreProfile ?
        getCoreProfileAttributes() :
        getLegacyProfileAttributes();

    // Format it as |variant| |attribs|
    int variant_size =
        getAttrListLength(selected_variant);
    int attrib_size = getAttrListLength(attrs);
    int numAttribsTotal = attrib_size + variant_size + 1; // for trailing 0

    NSOpenGLPixelFormatAttribute* newAttrs =
        malloc(sizeof(NSOpenGLPixelFormatAttribute) * numAttribsTotal);

    int variant_part_bytes =
        sizeof(NSOpenGLPixelFormatAttribute) * variant_size;
    int attribs_part_bytes =
        sizeof(NSOpenGLPixelFormatAttribute) * attrib_size;

    memcpy(newAttrs, selected_variant, variant_part_bytes);
    memcpy((char*)newAttrs + variant_part_bytes,
           attrs, attribs_part_bytes);
    newAttrs[numAttribsTotal - 1] = 0;

    void* finalizedFormat =
        [[NSOpenGLPixelFormat alloc] initWithAttributes: newAttrs];

    free(newAttrs);

    return finalizedFormat;
}

static bool sIsKeyValueAttrib(NSOpenGLPixelFormatAttribute attrib) {
    switch (attrib) {
        // These are the ones that take a value, according to the current
        // NSOpenGLPixelFormat docs
        case NSOpenGLPFAOpenGLProfile:
        case NSOpenGLPFAAuxBuffers:
        case NSOpenGLPFAColorSize:
        case NSOpenGLPFAAlphaSize:
        case NSOpenGLPFADepthSize:
        case NSOpenGLPFAStencilSize:
        case NSOpenGLPFAAccumSize:
        case NSOpenGLPFARendererID:
        case NSOpenGLPFAScreenMask:
            return true;
        default:
            return false;
    }
}

int getPixelFormatAttrib(int i, int _query) {
    NSOpenGLPixelFormatAttribute query =
        (NSOpenGLPixelFormatAttribute)_query;
    int size;
    const NSOpenGLPixelFormatAttribute* const* attrib_lists = getPixelFormatsAttributes(&size);
    int attributes_num = i % size;
    const NSOpenGLPixelFormatAttribute* attribs = attrib_lists[attributes_num];
    int res = 0;
    while (*attribs) {
        if (sIsKeyValueAttrib(*attribs)) {
            if (query == *attribs) {
                return attribs[1];
            }
            attribs += 2;
        } else {
            // these are boolean attribs.
            // their mere presence signals
            // that the query should return true.
            if (query == *attribs) {
                return 1;
            }
            attribs++;
        }
    }
    // return 0 if key not found---takes care of all boolean attribs,
    // and we depend on returning alpha=0 to make the default
    // config for GLSurfaceView happy.
    return 0;
}

void* nsCreateContext(void* format,void* share){
    NSOpenGLPixelFormat* frmt = (NSOpenGLPixelFormat*)format;
    return [[EmuGLContext alloc] initWithFormat:frmt shareContext:share];
}

void* nsGetLowLevelContext(void* context) {
    EmuGLContext* ctx = (EmuGLContext*)context;
    return ctx;
}

void nsCopyTexture(void* context, int from, int to, int width, int height) {
    EmuGLContext* ctx = (EmuGLContext*)context;

    if (glGetError() != GL_NO_ERROR) {
        // ignore
    }
    int tex1 = from;
    int tex2 = to;
    GLuint g_fb = 0;
    glGenFramebuffers(1, &g_fb);
    glBindFramebuffer(GL_FRAMEBUFFER, g_fb);
    glBindTexture(GL_TEXTURE_RECTANGLE, tex1);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_RECTANGLE, tex1, 0);
    if (glGetError() != GL_NO_ERROR) {
        return;
    }
    glBindTexture(GL_TEXTURE_2D, tex2);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1,
                           GL_TEXTURE_2D, tex2, 0);
    if (glGetError() != GL_NO_ERROR) {
        return;
    }
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, width, height);

    glBindTexture(GL_TEXTURE_RECTANGLE, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    if (glGetError() != GL_NO_ERROR) {
        return;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &g_fb);
}

void nsConvertVideoFrameToNV12Textures(void* context,
                                       void* iosurface,
                                       int Ytexd,
                                       int UVtexd) {
    EmuGLContext* ctx = (EmuGLContext*)context;

    CGLContextObj cgl_ctx = ctx.CGLContextObj;

    glEnable(GL_TEXTURE_RECTANGLE);

    IOSurfaceRef* surface = (IOSurfaceRef*)iosurface;

    GLsizei surface_w = (GLsizei)IOSurfaceGetWidth(surface);
    GLsizei surface_h = (GLsizei)IOSurfaceGetHeight(surface);

    if (!ctx->ytexForDecoder) {
        glGenTextures(1, &ctx->ytexForDecoder);
    }
    if (!ctx->uvtexForDecoder) {
        glGenTextures(1, &ctx->uvtexForDecoder);
    }
    GLuint Ytex = ctx->ytexForDecoder;
    GLuint UVtex = ctx->uvtexForDecoder;

    glBindTexture(GL_TEXTURE_RECTANGLE, Ytex);
    CGLError cglError = CGLTexImageIOSurface2D(
            cgl_ctx, GL_TEXTURE_RECTANGLE, GL_R8, surface_w, surface_h, GL_RED,
            GL_UNSIGNED_BYTE, surface, 0);

    if (cglError != kCGLNoError) {
        return;
    }

    glBindTexture(GL_TEXTURE_RECTANGLE, UVtex);
    cglError = CGLTexImageIOSurface2D(cgl_ctx, GL_TEXTURE_RECTANGLE, GL_RG8,
                                      surface_w / 2, surface_h / 2, GL_RG,
                                      GL_UNSIGNED_BYTE, surface, 1);

    if (cglError != kCGLNoError) {
        return;
    }
    glBindTexture(GL_TEXTURE_RECTANGLE, 0);

    nsCopyTexture(context, (int)Ytex, Ytexd, surface_w, surface_h);
    nsCopyTexture(context, (int)UVtex, UVtexd, surface_w/2, surface_h/2);
}

void  nsPBufferMakeCurrent(void* context,void* nativePBuffer,int level){
    EmuGLContext* ctx = (EmuGLContext *)context;
    NSOpenGLPixelBuffer* pbuff = (NSOpenGLPixelBuffer *)nativePBuffer;
    if(ctx == nil){
        [NSOpenGLContext clearCurrentContext];
    } else {
        if(pbuff != nil){
            [ctx preBind:1];
            [ctx setPixelBuffer:pbuff cubeMapFace:0 mipMapLevel:level currentVirtualScreen:0];
            [ctx makeCurrentContext];
        } else {
            // in this case, pbuffers deprecated and disabled.
            [ctx preBind:0];
            [ctx makeCurrentContext];
        }
    }
}

void nsWindowMakeCurrent(void* context,void* nativeWin){
    EmuGLContext* ctx = (EmuGLContext *)context;
    NSView* win = (NSView *)nativeWin;
    if(ctx == nil){
        [NSOpenGLContext clearCurrentContext];
    } else if (win != nil) {
        [ctx preBind:0];
        [ctx setView: win];
        [ctx makeCurrentContext];
    }
}

void nsSwapBuffers(){
    NSOpenGLContext* ctx = [NSOpenGLContext currentContext];
    if(ctx != nil){
        [ctx flushBuffer];
    }
}

void nsSwapInterval(int *interval){
    NSOpenGLContext* ctx = [NSOpenGLContext currentContext];
    if( ctx != nil){
        [ctx setValues:interval forParameter:NSOpenGLCPSwapInterval];
    }
}


void nsDestroyContext(void* context){
    EmuGLContext *ctx = (EmuGLContext*)context;
    if(ctx != nil){
        if (ctx->ytexForDecoder != 0) {
            glDeleteTextures(1, &ctx->ytexForDecoder);
            ctx->ytexForDecoder = 0;
        }
        if (ctx->uvtexForDecoder != 0) {
            glDeleteTextures(1, &ctx->uvtexForDecoder);
            ctx->uvtexForDecoder = 0;
        }
        [ctx release];
    }
}


void* nsCreatePBuffer(GLenum target,GLenum format,int maxMip,int width,int height){
    return [[NSOpenGLPixelBuffer alloc] initWithTextureTarget:target
                                        textureInternalFormat:format
                                        textureMaxMipMapLevel:maxMip
                                        pixelsWide:width pixelsHigh:height];

}

void nsDestroyPBuffer(void* pbuffer){
    NSOpenGLPixelBuffer *pbuf = (NSOpenGLPixelBuffer*)pbuffer;
    if(pbuf != nil){
        [pbuf release];
    }
}

bool nsGetWinDims(void* win,unsigned int* width,unsigned int* height){
    NSView* view = (NSView*)win;
    if(view != nil){
        NSRect rect = [view bounds];
        *width  = rect.size.width;
        *height = rect.size.height;
        return true;
    }
    return false;
}

bool  nsCheckColor(void* win,int colorSize){
    NSView* view = (NSView*)win;
   if(view != nil){
       NSWindow* wnd = [view window];
       if(wnd != nil){
           NSWindowDepth limit = [wnd depthLimit];
           NSWindowDepth defaultLimit = [NSWindow defaultDepthLimit];

           int depth = (limit != 0) ? NSBitsPerPixelFromDepth(limit):
                                      NSBitsPerPixelFromDepth(defaultLimit);
           return depth >= colorSize;

       }
   }
   return false;

}

void* nsGetLayer(void* win) {
    NSView* view = (NSView*)win;
    return view.layer;
}
