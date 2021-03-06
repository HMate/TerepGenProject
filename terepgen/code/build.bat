@echo off
REM encoding: OEM 852
REM Store and change the codepage to see chracters with accent
REM for /f "tokens=2 delims=:." %%x in ('chcp') do set cp=%%x
REM chcp 1252>nul

echo TerepGen compile starting at %TIME%

set projPath=%~dp0

cd /d %projPath%
REM load_msvc.bat set up the enviroment, so msvc cl.exe can be called
REM eg. it contains a similar line : call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x64
call "..\misc\load_msvc.bat"

set debugBuild= 0

if %debugBuild% == 1 (
echo Debug build
REM -MTd Multithreaded version of run-time library build and links to LIBCMT.lib in debug mode
REM -Od Disables optimization and build faster
REM -Z7 Produces an .obj with symbolic debugging information for debuggers
REM -D.. Sets the given variable for preprocessor directives
REM -EH Exceptions handling. s: only c++ exceptions c: extern "C" functions never throw exceptions
REM -W4 Turns on warnings -Wall turns on more warnings than -W4
REM -WX Treat warnings as errors
REM -Fe Name of exe file
set CommonCompilerFlags= -MTd -nologo -Od -Z7 -EHsc -DTEREPGEN_DEBUG=1 -DTEREPGEN_PERF=1^
 -W4 -WX -wd4201 -wd4100 -wd4505 -wd4189 -wd4239 -Feterepgen.exe -fp:fast

REM -opt:ref Eliminates functions and data that are not referenced
set CommonLinkerFlags= -incremental:no -opt:ref^
 user32.lib d3d11.lib dxgi.lib d3dcompiler.lib 
 
) else (
echo profile build
rem FAsc ?generates asesmbly sources
rem /d1reportSingleClassLayoutClassName prints ClassName's layout
rem linker /map for checking the alignment of memory structs
set CommonCompilerFlags= -MT -nologo -O2 -Z7 -EHsc -DTEREPGEN_DEBUG=0 -DTEREPGEN_PERF=1^
 -W4 -WX -wd4201 -wd4100 -wd4505 -wd4189 -wd4239 -Feterepgen.exe -fp:fast
set CommonLinkerFlags= -incremental:no -opt:ref^
 user32.lib d3d11.lib dxgi.lib d3dcompiler.lib 
)
 
REM List the cpp files to compile here
set CompiledFiles= ..\code\windows_terepgen.cpp ..\code\terepgen.cpp ..\code\generator\generator.cpp^
 ..\code\renderer\renderer.cpp

if not exist ..\build mkdir ..\build
pushd ..\build
cl %CommonCompilerFlags% %CompiledFiles% /link %CommonLinkerFlags%

if %debugBuild% == 1 (
    if not exist Debug mkdir Debug
    xcopy /Y /Q ..\data Debug
    xcopy /Y /Q terepgen.exe Debug
)

popd

REM Restore codepage
REM chcp %cp%>nul

echo TerepGen compile ended at %TIME%