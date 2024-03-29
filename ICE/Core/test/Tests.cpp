//
// Created by Thomas Ibanez on 16.11.20.
//
#define IMGUI_IMPL_OPENGL_LOADER_GL3W

#include <Logger.h>
#include <gtest/gtest.h>

using namespace ICE;

//#include "AssetBankTest.h"
#include "SceneTest.h"
#include "SceneGraphTest.h"
#include "ECSTest.h"

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    Logger::Log(Logger::DEBUG, "Core", "This is a debug message !");
    Logger::Log(Logger::VERBOSE, "Core", "This is a verbose message !");
    Logger::Log(Logger::INFO, "Core", "This is a info message !");
    Logger::Log(Logger::WARNING, "Core", "This is a warning message !");
    Logger::Log(Logger::ERROR, "Core", "This is a error message !");
    Logger::Log(Logger::FATAL, "Core", "This is a fatal message !");
    return RUN_ALL_TESTS();
}