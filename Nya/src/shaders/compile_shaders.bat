@echo off
set shaderDir=%~dp0

CALL "%shaderDir%glslc.exe" "%shaderDir%shader.vert" -o "%shaderDir%shader.vert.spv"
CALL "%shaderDir%glslc.exe" "%shaderDir%shader.frag" -o "%shaderDir%shader.frag.spv" 