Introduction: Welcome to the Dazel game engine project where we develop a game engine from the ground up using vulkan and C/C++ 
Goal: The goal of this project is to understand the fundamentals behind computer 3D graphics and how mathematical models can be used to create real time 3D simulations

Features: The engine is uses vulkan as the graphics API to produces graphics on the screen. Vulkan is verbose version of the popular openGL API that gives us developers 
more contorl over the GPU of our system, allowing us to squeeze out as much performance as possible from the GPU. the engine at the current moment has features like 
1.) The ability to import meshes or models from blender ( other 3D applications such 3D max, Maya etc  will be added soon )
2.) Camera movement using the axis angle formula and eurler angles 
3.) transformation (Rotations, scaling, translation) 
4.) basic lighting ( phong lighting model) 
5.) Normal maps ( this adds more detail to objects in the scene such as roughness and also allows for better lighting effect )

Note: This list will be frequently updated as more features and sub system are added to the code base

Current situation: The developement of the engine is currently on a pause as i am the sole developer involed in it at the time, but i can guarantee that the pace of 
development will pick  up right after my personal issues have been sorted out 

future goals : Adding features such as the following 
1.) Hot reloading 
2.) Cube maps 
3.) A more robost camera system 
4.) Physics engine 
5.) Custom UI system 
6.) Terrians and water simulation 
7.) 3D Gizmo 
8.) sky box
9.) Audio systems 
10.) job system 
11.) Advance lighting systems ( using advance lighting models )
12.) Tooling 
13.) Better build system for the code base 
14.) linux and mac os support 
15.) Utilization of other graphics API's such as Direct X3D and OpenGL
16.) Automation using Artificial intelligence 
17.) *Deep learning application (the symbol * means we may or may not support this depending on the situation at that time)

Note: This isn't an exhaustive list. There are more features that we will support on this project

Conclusion : It has been fun and challenging building this project but i a really greatfull for chosing this path of computer graphics as it is a lot important not only 
in games but in other multimedia content all around the globe in different spheres of profession. So join on the facinating journey of building a virtual world of our
own from the principles of math and physics that governs our currnet reality 

Note: To run the project you have to have vulkan installed on your machine and add the path to your machines enviromental path. Then build the code on VS Code using the batch files that are available on in the code base. Please make sure you set the build for the code to use the batch files from the code base. inorder for the shaders to
compile along side the compilation of the c++ code ( the vulkan sdk folder should be installed on the C drive of you machine "C:/VulkanSDK/1.3.275.0/" th path here is the 
exact way it is meant to be on your machine. please take note of the version of vulkan been used  "1.3.275.0" a different version might cause issues while compiling the shaders. 
