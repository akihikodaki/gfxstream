add_library(GLSnapshot GLSnapshot.cpp)
target_link_libraries(GLSnapshot PUBLIC apigen-codec-common)
target_include_directories(GLSnapshot PUBLIC
    ${GFXSTREAM_REPO_ROOT}/host/gl/glsnapshot
    ${GFXSTREAM_REPO_ROOT}/include
    ${GFXSTREAM_REPO_ROOT}/host
    ${GFXSTREAM_REPO_ROOT}/host/gl/OpenGLESDispatch)
