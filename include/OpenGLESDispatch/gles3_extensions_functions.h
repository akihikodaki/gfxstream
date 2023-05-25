// Auto-generated with: android/scripts/gen-entries.py --mode=funcargs stream-servers/gl/OpenGLESDispatch/gles3_extensions.entries --output=include/OpenGLESDispatch/gles3_extensions_functions.h
// DO NOT EDIT THIS FILE

#ifndef GLES3_EXTENSIONS_FUNCTIONS_H
#define GLES3_EXTENSIONS_FUNCTIONS_H

#define LIST_GLES3_EXTENSIONS_FUNCTIONS(X) \
  X(void, glVertexAttribIPointerWithDataSize, (GLuint indx, GLint size, GLenum type, GLsizei stride, const GLvoid* ptr, GLsizei dataSize), (indx, size, type, stride, ptr, dataSize)) \
  X(void, glPrimitiveRestartIndex, (GLuint index), (index)) \
  X(void, glTexBufferOES, (GLenum target, GLenum internalformat, GLuint buffer), (target, internalformat, buffer)) \
  X(void, glTexBufferRangeOES, (GLenum target, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size), (target, internalformat, buffer, offset, size)) \
  X(void, glTexBufferEXT, (GLenum target, GLenum internalformat, GLuint buffer), (target, internalformat, buffer)) \
  X(void, glTexBufferRangeEXT, (GLenum target, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size), (target, internalformat, buffer, offset, size)) \
  X(void, glEnableiEXT, (GLenum cap, GLuint index), (cap, index)) \
  X(void, glDisableiEXT, (GLenum cap, GLuint index), (cap, index)) \
  X(void, glBlendEquationiEXT, (GLuint buf, GLenum mode), (buf, mode)) \
  X(void, glBlendEquationSeparateiEXT, (GLuint buf, GLenum modeRGB, GLenum modeAlpha), (buf, modeRGB, modeAlpha)) \
  X(void, glBlendFunciEXT, (GLuint buf, GLenum sfactor, GLenum dfactor), (buf, sfactor, dfactor)) \
  X(void, glBlendFuncSeparateiEXT, (GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha), (buf, srcRGB, dstRGB, srcAlpha, dstAlpha)) \
  X(void, glColorMaskiEXT, (GLuint buf, GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha), (buf, red, green, blue, alpha)) \
  X(GLboolean, glIsEnablediEXT, (GLenum cap, GLuint index), (cap, index)) \


#endif  // GLES3_EXTENSIONS_FUNCTIONS_H
