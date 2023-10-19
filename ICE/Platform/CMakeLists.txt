cmake_minimum_required(VERSION 3.19)
project(platform)

message(STATUS "Building ${PROJECT_NAME} module")

add_library(${PROJECT_NAME} STATIC)

if(APPLE)
	set(EXTRA_SRC OSX/dialog.mm)
elseif(WIN32)
	set(EXTRA_SRC Win32/dialog.cpp)
else()
	link_libraries(-lstdc++fs)
	set(EXTRA_SRC Linux/dialog.cpp)
endif()

target_sources(${PROJECT_NAME} PRIVATE
    FileUtils.cpp
    ${EXTRA_SRC}
)

target_link_libraries(${PROJECT_NAME} 
	PUBLIC
	util
)

target_include_directories(${PROJECT_NAME} PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

enable_testing()
#add_subdirectory(test)
