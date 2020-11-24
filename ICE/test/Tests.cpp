//
// Created by Thomas Ibanez on 16.11.20.
//
#include <Scene/Entity.h>
#include <Scene/TransformComponent.h>
#include <Scene/Scene.h>

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
    return 0;
}