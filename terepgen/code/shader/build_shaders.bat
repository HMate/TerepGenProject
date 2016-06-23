@echo off
REM this batch file compiles the shaders needed by the terrain generator
REM Set the path to the fxc.exe dx shader compiler
set fxc="C:\Program Files (x86)\Windows Kits\8.1\bin\x64\fxc.exe"
set outputDir=..\..\data\

pushd "Y:\code\shader"
call %fxc% /E VShader /T vs_4_0 /Fo %outputDir%terrain_vs.fxc shaders.hlsl
call %fxc% /E TerrainPShader /T ps_4_0 /Fo %outputDir%terrain_ps.fxc shaders.hlsl
call %fxc% /E LinePShader /T ps_4_0 /Fo %outputDir%line_ps.fxc shaders.hlsl

call %fxc% /E BackgroundVShader /T vs_4_0 /Fo %outputDir%background_vs.fxc shader_background.hlsl
call %fxc% /E BackgroundPShader /T ps_4_0 /Fo %outputDir%background_ps.fxc shader_background.hlsl
popd