#README

![Image of BlockGame-v0](http://i1218.photobucket.com/albums/dd401/222464/PGE_LOGO.png)

## PGE - Parallel Game Engine

An engine intended for fast and pretty 3D artficial intelligence experiments.

PGE is currently set up to interface with the OpenAI Gym (https://github.com/openai/gym). It allows you to train your reinforcement learning agents using the OpenAI Gym API.

### SETUP:

To get started with PGE, you need your favorite compiler and CMake (http://www.cmake.org/cmake/resources/software.html).

You will also need to install 2 libraries: Bullet Physics and SFML.

Get the libraries here:
https://github.com/bulletphysics/bullet3/releases
http://www.sfml-dev.org/download.php

You may also need to generate an OpenGL extension loader with glLoadGen, if the existing include ones don't work for you: https://bitbucket.org/alfonse/glloadgen/wiki/Home 

Once you have these libraries all set up, set CMake's source code directory to the PGE directory (the one that contains the /source folder as well as a CMakeLists.txt).

Set CMake's build directory to the same directory as in the previous step. Optionally, you can also set it to a folder of your choice.

Then press configure, and choose your compiler.

It will likely error if you did not install some of the libraries to a standard location. If this happens, no fear!

You can specify the paths where CMake looks manually. They will appear in red if they need to be set in the CMake GUI.

Typically, you can manually define an entry in CMake with the format `<library name here>_ROOT`, and point that to the root directory of the library to automatically set multiple paths.
When eventually the configuration does not result in errors you can hit generate. This will generate files necessary for your compiler.

To use the engine with the OpenAI Gym, you will need to have the Gym installed. Follow the instructions on the OpenAI Gym site to do this.
Once installed, all you need to do to use the PGE environments with the Gym is copy the PGE executable to the root directory (where CMakeLists.txt is), and create your Python experiment code such that it also starts from this directory.
You can then import pypge (Python PGE), which will automatically register the environment with the Gym. The python bindings support Python 2.7, but 3 should work as well.

Currently, there is one environment, but there are more to come!

| Name           | Description                                                                                                                      | State                                                                   | Action                    |
|----------------|----------------------------------------------------------------------------------------------------------------------------------|-------------------------------------------------------------------------|---------------------------|
| BlockGame-v0   | A simple block stacking game in 3D. The agent must get as high up as possible, by moving blocks to build structures.             | 9 integers in range [0, inf) (3x3 square of block heights around agent) | 8 discrete actions [0, 8) |
| CartPole3D-v0  | Cart pole extended to 3 dimensions. The agent must balance an inverted pendulum on a cart that can move horizontally in X and Z. | 8 real values. Approximate range of (-1.0, 1.0).                        | 2 real values in [-1, 1]  |
| Quadruped3D-v0 | A mammalian quadruped with body twist joint. The agent must move the quadruped as far from the starting point as possible.       | 34 real values. Approximate range of (-1.0, 1.0).                       | 27 real values in [-1, 1] |

### Images

BlockGame-v0:

![Image of BlockGame-v0](http://i1218.photobucket.com/albums/dd401/222464/blockgame-v0-1.png)

CartPole3D-v0:

![Image of CartPole3D-v0](http://i1218.photobucket.com/albums/dd401/222464/cartPole3D1.png)

Quadruped3D-v0:

![Image of Quadruped3D-v0](http://i1218.photobucket.com/albums/dd401/222464/quadruped1.png)