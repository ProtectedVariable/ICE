cmake_minimum_required(VERSION 3.19)
project(ICE)

message(STATUS "Building ${PROJECT_NAME}")

add_subdirectory(Assets)
add_subdirectory(Components)
add_subdirectory(Core)
add_subdirectory(Entity)
add_subdirectory(Graphics)
add_subdirectory(GraphicsAPI)
add_subdirectory(IO)
add_subdirectory(Math)
add_subdirectory(Multithreading)
add_subdirectory(Platform)
add_subdirectory(Scene)
add_subdirectory(Storage)
add_subdirectory(System)
add_subdirectory(Util)

set(ICE_LIBS assets core graphics graphics_api io math platform scene storage system util components entity)
add_library(${PROJECT_NAME} INTERFACE)

if(APPLE)
	target_link_libraries(${PROJECT_NAME} 
		INTERFACE 
		glfw
		${COCOA_LIBRARY} 
		${OPENGL_LIBRARY} 
		${IOKIT_LIBRARY} 
		${ICE_LIBS}
	)
elseif(WIN32)
	target_link_libraries(${PROJECT_NAME} 
		INTERFACE 
		glfw
		${ICE_LIBS}
	)
else()
	find_package(PkgConfig REQUIRED)

	pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

	find_package(OpenGL REQUIRED)

	target_include_directories(${PROJECT_NAME} INTERFACE ${GTK3_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIRS})
	target_compile_options(${PROJECT_NAME} INTERFACE ${GTK3_CFLAGS_OTHER})

	target_link_libraries(${PROJECT_NAME}
		INTERFACE
		glfw
		${GTK3_LIBRARIES}
		${OPENGL_LIBRARIES}
		stdc++fs
		${ICE_LIBS}
	)
endif()
