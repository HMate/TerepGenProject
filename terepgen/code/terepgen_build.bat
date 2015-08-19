@echo off
REM encoding: OEM 852
REM Store and change the codepage to see chracters with accent
REM for /f "tokens=2 delims=:." %%x in ('chcp') do set cp=%%x
REM chcp 1252>nul

echo TerepGen compile starting at %TIME%

cd /d Y:\code
call "..\misc\load_msvc.bat"

REM -MTd Multithreaded version of run-time library build and links to LIBCMT.lib
REM -Od Disables optimization and build faster
REM -Z7 Produces an .obj with symbolic debugging information for debuggers
REM -D.. Sets the given variable for preprocessor directives
set CommonCompilerFlags= -MTd -nologo -Od -Z7 -EHsc -DTEREPGEN_DEBUG=1
REM -opt:ref Eliminates functions and data that are not referenced
set CommonLinkerFlags= -incremental:no -opt:ref user32.lib d3d11.lib dxgi.lib d3dcompiler.lib

REM List the cpp files to compile here
set CompiledFiles= ..\code\windows_terepgen.cpp

if not exist ..\build mkdir ..\build
pushd ..\build
cl %CommonCompilerFlags% %CompiledFiles% /link %CommonLinkerFlags%
popd

REM Restore codepage
REM chcp %cp%>nul

echo TerepGen compile ended at %TIME%