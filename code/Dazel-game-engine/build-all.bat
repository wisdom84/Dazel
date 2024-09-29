@ECHO OFF
ECHO "Building everything"
PUSHD engine\src\Renderer\vulkan\shaders\shader_files
CALL compile_shaders.bat
POPD
IF %ERRORLEVEL% NEQ 0 (echo Error:%ERRORLEVEL% && exit)
ECHO "shaders compiled successfully"
PUSHD engine
CALL build.bat
POPD 
IF %ERRORLEVEL% NEQ 0 (echo Error:%ERRORLEVEL% && exit)
PUSHD testbed
CALL build.bat
POPD 
IF %ERRORLEVEL% NEQ 0 (echo Error:%ERRORLEVEL% && exit)
@REM REM Engine
@REM make -f "Makefile.engine.windows.mak" all
@REM IF %ERRORLEVEL% NEQ 0 (echo Error:%ERRORLEVEL% && exit)

@REM REM Testbed 
@REM make -f "Makefile.testbed.windows.mak" all
@REM IF %ERRORLEVEL% NEQ 0 (echo Error:%ERRORLEVEL% && exit)

ECHO "All assemblies built successfully"
