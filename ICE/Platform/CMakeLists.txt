cmake_minimum_required(VERSION 3.19)
project(platform)

message(STATUS "Building ${PROJECT_NAME} module")

add_library(${PROJECT_NAME} STATIC)

if(APPLE)
	set(EXTRA_SRC OSX/dialog.mm)
elseif(WIN32)
	set(EXTRA_SRC Win32/dialog.cpp)
else()
	set(EXTRA_SRC Linux/dialog.cpp)
	# GTK3 headers live under /usr/include/gtk-3.0 (+ glib/cairo/pango) and are
	# only discoverable through pkg-config; resolve them here so dialog.cpp compiles.
	find_package(PkgConfig REQUIRED)
	pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
	set(EXTRA_INC ${GTK3_INCLUDE_DIRS})
	set(EXTRA_LIBS ${GTK3_LIBRARIES} stdc++fs)
	set(EXTRA_LIBDIRS ${GTK3_LIBRARY_DIRS})
endif()

target_sources(${PROJECT_NAME} PRIVATE
    FileUtils.cpp
    ${EXTRA_SRC}
)

if(EXTRA_LIBDIRS)
	target_link_directories(${PROJECT_NAME} PUBLIC ${EXTRA_LIBDIRS})
endif()

target_link_libraries(${PROJECT_NAME}
	PUBLIC
	util
	${EXTRA_LIBS}
)

target_include_directories(${PROJECT_NAME} PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
	${EXTRA_INC}
 )

enable_testing()
#add_subdirectory(test)
