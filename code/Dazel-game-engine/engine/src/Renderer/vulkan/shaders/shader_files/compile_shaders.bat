@ECHO OFF
ECHO Compiling shaders 
C:/VulkanSDK/1.3.275.0/Bin/glslangValidator.exe shader.vert -V -o  cd/../../../../../../../bin/assets/shader_bin/vert.spv
C:/VulkanSDK/1.3.275.0/Bin/glslangValidator.exe shader.frag -V -o  cd/../../../../../../../bin/assets/shader_bin/frag.spv

C:/VulkanSDK/1.3.275.0/Bin/glslangValidator.exe UI_shader.vert -V -o cd/../../../../../../../bin/assets/shader_bin/UIvert.spv
C:/VulkanSDK/1.3.275.0/Bin/glslangValidator.exe UI_shader.frag -V -o cd/../../../../../../../bin/assets/shader_bin/UIfrag.spv

C:/VulkanSDK/1.3.275.0/Bin/glslangValidator.exe light_shader.vert -V -o  cd/../../../../../../../bin/assets/shader_bin/light_vert.spv
C:/VulkanSDK/1.3.275.0/Bin/glslangValidator.exe light_shader.frag -V -o  cd/../../../../../../../bin/assets/shader_bin/light_frag.spv


