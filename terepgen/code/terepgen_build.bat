@echo off

echo TerepGen compile starting at %TIME%

cd /d Y:\code
REM call "D:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" x64
call "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" x64

REM -Od Disables optimization and build faster
REM -MTd Multithreaded version of run-time library build and links to LIBCMT.lib
REM -Z7 Produces an .obj with symbolic debugging information for debuggers
set CommonCompilerFlags= -MTd -nologo -Od -Z7 -EHsc
REM -opt:ref Eliminates functions and data that are not referenced
set CommonLinkerFlags= -incremental:no -opt:ref user32.lib d3d11.lib d3dcompiler.lib

if not exist ..\build mkdir ..\build
pushd ..\build
cl %CommonCompilerFlags% ..\code\windows_terepgen.cpp ..\code\terepgen_terrain.cpp^
 ..\code\terepgen_terrain_renderer.cpp /link %CommonLinkerFlags%
popd

echo TerepGen compile ended at %TIME%