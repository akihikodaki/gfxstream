// Generated Code - DO NOT EDIT !!
// generated by 'emugen'


#include <string.h>
#include "renderControl_server_context.h"


#include <stdio.h>

int renderControl_server_context_t::initDispatchByName(void *(*getProc)(const char *, void *userData), void *userData)
{
	rcGetRendererVersion = (rcGetRendererVersion_server_proc_t) getProc("rcGetRendererVersion", userData);
	rcGetEGLVersion = (rcGetEGLVersion_server_proc_t) getProc("rcGetEGLVersion", userData);
	rcQueryEGLString = (rcQueryEGLString_server_proc_t) getProc("rcQueryEGLString", userData);
	rcGetGLString = (rcGetGLString_server_proc_t) getProc("rcGetGLString", userData);
	rcGetNumConfigs = (rcGetNumConfigs_server_proc_t) getProc("rcGetNumConfigs", userData);
	rcGetConfigs = (rcGetConfigs_server_proc_t) getProc("rcGetConfigs", userData);
	rcChooseConfig = (rcChooseConfig_server_proc_t) getProc("rcChooseConfig", userData);
	rcGetFBParam = (rcGetFBParam_server_proc_t) getProc("rcGetFBParam", userData);
	rcCreateContext = (rcCreateContext_server_proc_t) getProc("rcCreateContext", userData);
	rcDestroyContext = (rcDestroyContext_server_proc_t) getProc("rcDestroyContext", userData);
	rcCreateWindowSurface = (rcCreateWindowSurface_server_proc_t) getProc("rcCreateWindowSurface", userData);
	rcDestroyWindowSurface = (rcDestroyWindowSurface_server_proc_t) getProc("rcDestroyWindowSurface", userData);
	rcCreateColorBuffer = (rcCreateColorBuffer_server_proc_t) getProc("rcCreateColorBuffer", userData);
	rcOpenColorBuffer = (rcOpenColorBuffer_server_proc_t) getProc("rcOpenColorBuffer", userData);
	rcCloseColorBuffer = (rcCloseColorBuffer_server_proc_t) getProc("rcCloseColorBuffer", userData);
	rcSetWindowColorBuffer = (rcSetWindowColorBuffer_server_proc_t) getProc("rcSetWindowColorBuffer", userData);
	rcFlushWindowColorBuffer = (rcFlushWindowColorBuffer_server_proc_t) getProc("rcFlushWindowColorBuffer", userData);
	rcMakeCurrent = (rcMakeCurrent_server_proc_t) getProc("rcMakeCurrent", userData);
	rcFBPost = (rcFBPost_server_proc_t) getProc("rcFBPost", userData);
	rcFBSetSwapInterval = (rcFBSetSwapInterval_server_proc_t) getProc("rcFBSetSwapInterval", userData);
	rcBindTexture = (rcBindTexture_server_proc_t) getProc("rcBindTexture", userData);
	rcBindRenderbuffer = (rcBindRenderbuffer_server_proc_t) getProc("rcBindRenderbuffer", userData);
	rcColorBufferCacheFlush = (rcColorBufferCacheFlush_server_proc_t) getProc("rcColorBufferCacheFlush", userData);
	rcReadColorBuffer = (rcReadColorBuffer_server_proc_t) getProc("rcReadColorBuffer", userData);
	rcUpdateColorBuffer = (rcUpdateColorBuffer_server_proc_t) getProc("rcUpdateColorBuffer", userData);
	rcOpenColorBuffer2 = (rcOpenColorBuffer2_server_proc_t) getProc("rcOpenColorBuffer2", userData);
	rcCreateClientImage = (rcCreateClientImage_server_proc_t) getProc("rcCreateClientImage", userData);
	rcDestroyClientImage = (rcDestroyClientImage_server_proc_t) getProc("rcDestroyClientImage", userData);
	rcSelectChecksumHelper = (rcSelectChecksumHelper_server_proc_t) getProc("rcSelectChecksumHelper", userData);
	rcCreateSyncKHR = (rcCreateSyncKHR_server_proc_t) getProc("rcCreateSyncKHR", userData);
	rcClientWaitSyncKHR = (rcClientWaitSyncKHR_server_proc_t) getProc("rcClientWaitSyncKHR", userData);
	rcFlushWindowColorBufferAsync = (rcFlushWindowColorBufferAsync_server_proc_t) getProc("rcFlushWindowColorBufferAsync", userData);
	rcDestroySyncKHR = (rcDestroySyncKHR_server_proc_t) getProc("rcDestroySyncKHR", userData);
	rcSetPuid = (rcSetPuid_server_proc_t) getProc("rcSetPuid", userData);
	rcUpdateColorBufferDMA = (rcUpdateColorBufferDMA_server_proc_t) getProc("rcUpdateColorBufferDMA", userData);
	rcCreateColorBufferDMA = (rcCreateColorBufferDMA_server_proc_t) getProc("rcCreateColorBufferDMA", userData);
	rcWaitSyncKHR = (rcWaitSyncKHR_server_proc_t) getProc("rcWaitSyncKHR", userData);
	rcCompose = (rcCompose_server_proc_t) getProc("rcCompose", userData);
	rcCreateDisplay = (rcCreateDisplay_server_proc_t) getProc("rcCreateDisplay", userData);
	rcDestroyDisplay = (rcDestroyDisplay_server_proc_t) getProc("rcDestroyDisplay", userData);
	rcSetDisplayColorBuffer = (rcSetDisplayColorBuffer_server_proc_t) getProc("rcSetDisplayColorBuffer", userData);
	rcGetDisplayColorBuffer = (rcGetDisplayColorBuffer_server_proc_t) getProc("rcGetDisplayColorBuffer", userData);
	rcGetColorBufferDisplay = (rcGetColorBufferDisplay_server_proc_t) getProc("rcGetColorBufferDisplay", userData);
	rcGetDisplayPose = (rcGetDisplayPose_server_proc_t) getProc("rcGetDisplayPose", userData);
	rcSetDisplayPose = (rcSetDisplayPose_server_proc_t) getProc("rcSetDisplayPose", userData);
	rcSetColorBufferVulkanMode = (rcSetColorBufferVulkanMode_server_proc_t) getProc("rcSetColorBufferVulkanMode", userData);
	rcReadColorBufferYUV = (rcReadColorBufferYUV_server_proc_t) getProc("rcReadColorBufferYUV", userData);
	rcIsSyncSignaled = (rcIsSyncSignaled_server_proc_t) getProc("rcIsSyncSignaled", userData);
	rcCreateColorBufferWithHandle = (rcCreateColorBufferWithHandle_server_proc_t) getProc("rcCreateColorBufferWithHandle", userData);
	rcCreateBuffer = (rcCreateBuffer_server_proc_t) getProc("rcCreateBuffer", userData);
	rcCloseBuffer = (rcCloseBuffer_server_proc_t) getProc("rcCloseBuffer", userData);
	rcSetColorBufferVulkanMode2 = (rcSetColorBufferVulkanMode2_server_proc_t) getProc("rcSetColorBufferVulkanMode2", userData);
	rcMapGpaToBufferHandle = (rcMapGpaToBufferHandle_server_proc_t) getProc("rcMapGpaToBufferHandle", userData);
	rcCreateBuffer2 = (rcCreateBuffer2_server_proc_t) getProc("rcCreateBuffer2", userData);
	rcMapGpaToBufferHandle2 = (rcMapGpaToBufferHandle2_server_proc_t) getProc("rcMapGpaToBufferHandle2", userData);
	rcFlushWindowColorBufferAsyncWithFrameNumber = (rcFlushWindowColorBufferAsyncWithFrameNumber_server_proc_t) getProc("rcFlushWindowColorBufferAsyncWithFrameNumber", userData);
	rcSetTracingForPuid = (rcSetTracingForPuid_server_proc_t) getProc("rcSetTracingForPuid", userData);
	return 0;
}

