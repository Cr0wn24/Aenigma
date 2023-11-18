@echo off

REM -Zi:     Produce debugging information
REM -FC:     Use full code path in compilation output
REM -nologo: Don't print msvc banner
REM -Oi:     Enable compiler intrinsics 
REM -GR-:    Disable run-time type information
REM -EHa:    Disable exception handling
REM -MTd     Don't use multithreaded DLL (for compatibility)
REM -Gm-     Minimal rebuild

SET common_warnings=-W4 -WX -wd4201 -wd4100 -wd4189 -wd4505 -wd4456 -wd4101 -wd4706
SET common_flags=%common_warnings% %common_defines% -Gm- -fp:fast -MTd -EHa- -GR- -Od -Oi -Zi -FC -nologo
set common_linker_flags= -incremental:no -opt:ref user32.lib gdi32.lib kernel32.lib winmm.lib

if not exist ..\build\debug\ mkdir ..\build\debug\
pushd ..\build\debug\

ECHO --- Building the game ---
cl %common_defines% %common_flags% ..\..\src\test\ui\main.c -Fe:aenigma.dll -LD -link /EXPORT:GameUpdateAndRender

ECHO --- Building the platform layer ---
cl %common_defines% %common_flags% ..\..\src\win32_main.c -Fe:aenigma_app.exe -link %common_linker_flags%

SET var=%cd%
ECHO %var%

popd