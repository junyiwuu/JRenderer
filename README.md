# J-Renderer

![License: MIT](https://img.shields.io/badge/License-MIT-green.svg)

Personal Vulkan engine project made in C++.


# Overview:


https://github.com/user-attachments/assets/249f8700-3ac5-4825-8e3a-4f90043dea58



# Features
- Image Based Lighting
- Physically based shading
- HDR Skyboxes
- Archball camera
- Multisample Anti-aliasing (MSAA)
- Tangent space normal mapping
- Built-in ImGUI interface for real-time tweaking material properties
- backend

**To do list (updating)**

- Lighting System
- Shadowmap
- Exposure based tone mapping
- ...


# Install (Linux):
**Currently only support Linux**
1. Install Vulkan SDK from [here](https://vulkan.lunarg.com/).
2. Clone the repository.
```baseh
git clone --recursive https://github.com/junyiwuu/JRenderer.git
```  

3. Enter project folder.
```bash
cd JRenderer
```

4. Build.
```bash
cmake -B build 
cmake --build build -j$(nproc)
```

5. Launch the program.
``` bash
cd build
./JRenderer
```


# Dependencies:
- ASSIMP
- Dear ImGui
- GLFW
- stb_image
- KTX


# References
The list below contains the resources that I found extremely useful while developing this project. I am sharing them here and hope they may also benefit others who are interested in building their own Vulkan engine.
- [Khronos Vulkan Tutorial](https://gpx1000.github.io/Vulkan-Site/tutorial/latest/00_Introduction.html): The official Vulkan tutorial (but the old version). I started from here and went through at least twice. 
- [Vulkan Lecture Series](https://www.youtube.com/playlist?list=PLmIqTlJ6KsE1Jx5HV4sd2jOe3V1KMHHgn): Excellent explanations of Vulkan concepts.
- [Brendan Galea's tutorials](https://www.youtube.com/watch?v=Y9U9IE0gVHA&list=PL8327DO66nu9qYVKLDmdLW_84-yE4auCR&index=1): Another excellent explanations of Vulkan concepts.
- [3D Graphic Rendering Cookbook Second Edition](https://github.com/PacktPublishing/3D-Graphics-Rendering-Cookbook-Second-Edition): A great resource for understanding what to do next and how to approach it.
- [Sacha Willems Vulkan examples](https://github.com/SaschaWillems/Vulkan): Comprehensive Vulkan examples.
- [Cherno's C++ series](https://www.youtube.com/watch?v=18c3MTX0PK0&list=PLlrATfBNZ98dudnM48yfGUldqGD0S4FFb): Since I was learning C++ alongside this project, this series helps me a lot on C++ knowledge
- [PBRT 4](https://pbr-book.org/4ed/contents): : Fill gaps in graphics knowledge.



