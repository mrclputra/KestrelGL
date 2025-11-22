# KestrelGL

lightweight C++ OpenGL framework for loading and viewing 3D models
uses ASSIMP and ImGUI

vcpkg + cmake

made this project for the learning experience

TODO:
- implement loading single .HDRI files for skybox
- implement proper irradiance maps and specular maps from said HDRI map
- send generated irradiance and specular maps to model fragment shader
- process said maps in shader, for proper diffuse and reflections

- cleanup App()
- organize runtime updates into own classes/functions

- expand ImGUI options; toggle shader frag view types
- pannable orbit camera
- switchable to free camera; through imgui

- free load any skybox HDRI file during runtime (like current models)

- switch to bindless textures
- refactor OOP for more complex scenes (ObjectManager?)