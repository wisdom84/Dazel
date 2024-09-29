@ECHO OFF
SetLocal EnableDelayedExpansion
SET cppfilenames=
for /R %%f in (*.cpp) do (
   SET cppfilenames=!cppfilenames! %%f
)
SET assembly=testbed
SET compilerflags=-g
SET includeflags=-Isrc -I../engine/src/
SET linkerflags=-lengine.lib -L../bin/
SET defines=-DKIMPORT 
ECHO "Building %assembly%"
clang %cppfilenames% %compilerflags% -o ../bin/%assembly%.exe %defines% %includeflags% %linkerflags%