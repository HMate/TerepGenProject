@echo off
REM encoding: OEM 852
REM Store and change the codepage to see chracters with accent
REM for /f "tokens=2 delims=:." %%x in ('chcp') do set cp=%%x
REM chcp 1252>nul

echo TerepGen compile starting at %TIME%

cd /d Y:\code
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
set CommonCompilerFlags= -MTd -nologo -Od -Z7 -EHsc -DTEREPGEN_DEBUG=1 -W4 -WX -wd4201 -wd4100 -wd4505 -wd4189

REM -opt:ref Eliminates functions and data that are not referenced
set CommonLinkerFlags= -incremental:no -opt:ref^
 user32.lib d3d11.lib dxgi.lib d3dcompiler.lib 
 
) else (
echo Release build
set CommonCompilerFlags= -MT -nologo -O2 -EHsc -DTEREPGEN_DEBUG=0 -W4 -WX -wd4201 -wd4100 -wd4505 -wd4189
set CommonLinkerFlags= -incremental:no -opt:ref^
 user32.lib d3d11.lib dxgi.lib d3dcompiler.lib 
)
 
REM List the cpp files to compile here
set CompiledFiles= ..\code\windows_terepgen.cpp

if not exist ..\build mkdir ..\build
pushd ..\build
cl %CommonCompilerFlags% %CompiledFiles% /link %CommonLinkerFlags%
popd

REM Restore codepage
REM chcp %cp%>nul

echo TerepGen compile ended at %TIME%