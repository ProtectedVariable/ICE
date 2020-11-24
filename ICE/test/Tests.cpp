//
// Created by Thomas Ibanez on 16.11.20.
//
#include <Scene/Entity.h>
#include <Scene/TransformComponent.h>
#include <Scene/Scene.h>
#include <Util/Logger.h>

using namespace ICE;

//TODO: Add a real testing library
int main(void) {
    Entity e;
    assert(!e.hasComponent<TransformComponent>());
    TransformComponent tr;
    e.addComponent(tr);
    assert(e.hasComponent<TransformComponent>());
    assert(e.getComponent<TransformComponent>() == &tr);
    Scene s(nullptr);
    assert(s.getByID("my") == nullptr);
    s.addEntity("root", "my", e);
    assert(s.getByID("my") != nullptr);
    assert(s.getByID("my")->entity == &e);
    Logger::Log(Logger::DEBUG, "Core", "This is a debug message !");
    Logger::Log(Logger::VERBOSE, "Core", "This is a verbose message !");
    Logger::Log(Logger::INFO, "Core", "This is a info message !");
    Logger::Log(Logger::WARNING, "Core", "This is a warning message !");
    Logger::Log(Logger::ERROR, "Core", "This is a error message !");
    Logger::Log(Logger::FATAL, "Core", "This is a fatal message !");
    Logger::Log(Logger::VERBOSE, "Core", "Creating context...");
    return 0;
}