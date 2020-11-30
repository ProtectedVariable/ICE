# ICE

ICE is a 3D general purpose game engine written in C++. The aim of the project is to have an engine good enough to create all kind of games in an easy manner, but not to compete with the big names (unity/unreal).

## Getting started

Clone the repo with `git clone https://github.com/ProtectedVariable/ICE`

Create a `build/` directory in `ICE`

Download the glfw3 binaries you need for your OS from https://www.glfw.org/download.html and copy them in ICE/libs/glfw/

### Windows

In windows you should have:
- `ICE/libs/glfw/glfw3.dll`
- `ICE/libs/glfw/glfw3.lib`
- `ICE/libs/glfw/glfw3dll.lib`


You can now CMake GUI to configure a VS project (set Source dir: `ICE/`, set build sir: `ICE/build`)
- Press `Configure`
- Press `Generate`
- Press `Open Project`

In visual studio set `ICE` as the startup project and build :)

### OSX

In OSX you should have:
- `ICE/libs/glfw/libglfw.3.dylib`
- `ICE/libs/glfw/libglfw3.a`

Open a terminal in `ICE`
- `cd build`
- `cmake ..`
- `make`

This will generate an executable `ICE` in `ICE/cmake-build-debug/` :)

### Linux

TODO :)

## ECS Architecture

The engine is built with an Entity-Component-System architecture. This allow to have a vast array of behaviours on the different objects in the game very easily

## Roadmap

The [trello board](https://trello.com/b/Jstm3EL9/ice) of the project is available. This is a rough estimate of the progress done on the engine.
