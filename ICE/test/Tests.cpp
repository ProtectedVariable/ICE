//
// Created by Thomas Ibanez on 16.11.20.
//
#include <Scene/Entity.h>
#include <Scene/TransformComponent.h>

using namespace ICE;

//TODO: Add a real testing library
int main(void) {
    Entity e;
    assert(!e.hasComponent<TransformComponent>());
    TransformComponent tr;
    e.addComponent(tr);
    assert(e.hasComponent<TransformComponent>());
    assert(e.getComponent<TransformComponent>() == &tr);
    return 0;
}