@ECHO OFF
SetLocal EnableDelayedExpansion
SET cppfilenames=
for /R %%f in (*.cpp) do (
   SET cppfilenames=!cppfilenames! %%f
)
SET assembly=engine
SET compilerflags=-g -shared -Wno-vla -Wc++17-extensions
SET includeflags=-Isrc -I%VULKAN_SDK%/Include
SET linkerflags=-luser32 -lvulkan-1 -L%VULKAN_SDK%/Lib
SET defines=-DKEXPORT -D_CRT_SECURE_NO_WARNINGS
ECHO "Building %assembly%"
clang %cppfilenames% %compilerflags% -o ../bin/%assembly%.dll %defines% %includeflags% %linkerflags%





