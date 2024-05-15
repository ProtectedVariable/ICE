cmake_minimum_required(VERSION 3.19)
project(graphics_api_OpenGL)

message(STATUS "Building ${PROJECT_NAME} module")

add_library(${PROJECT_NAME} STATIC)

target_sources(${PROJECT_NAME} PRIVATE
		src/OpenGLVertexArray.cpp
		src/OpenGLContext.cpp
		src/OpenGLRendererAPI.cpp
		src/OpenGLBuffers.cpp
		src/OpenGLShader.cpp
		src/OpenGLFramebuffer.cpp
		src/OpenGLTexture2D.cpp
		src/OpenGLTextureCube.cpp
		${GL3W_SRC}
)

target_link_libraries(${PROJECT_NAME}
	PUBLIC
	graphics_api
	graphics
)

target_include_directories(${PROJECT_NAME} PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/include)

enable_testing()
#add_subdirectory(test)
