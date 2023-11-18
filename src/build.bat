@echo off

REM -Zi:     Produce debugging information
REM -FC:     Use full code path in compilation output
REM -nologo: Don't print msvc banner
REM -Oi:     Enable compiler intrinsics 
REM -GR-:    Disable run-time type information
REM -EHa:    Disable exception handling
REM -MTd     Don't use multithreaded DLL (for compatibility)
REM -Gm-     Minimal rebuild

set common_warnings=-W4 -WX -wd4201 -wd4100 -wd4189 -wd4505 -wd4456 -wd4101
set common_flags=%common_warnings% %common_defines% -Gm- -fp:fast -MTd -EHa- -GR- -O2 -Oi -Zi -FC -nologo
set common_linker_flags= -incremental:no -opt:ref user32.lib gdi32.lib kernel32.lib winmm.lib

if not exist ..\build\debug\ mkdir ..\build\debug\
pushd ..\build\debug\

echo --- Building the game ---
cl %common_defines% %common_flags% ..\..\src\aenigma.c -Fe:aenigma.dll -LD -link /EXPORT:GameUpdateAndRender

echo --- Building the platform layer ---
cl %common_defines% %common_flags% ..\..\src\win32_main.c -Fe:aenigma_app.exe -link %common_linker_flags% -DEBUG:FULL

echo Build finished.

popd