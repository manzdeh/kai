@echo off

IF NOT EXIST bin mkdir bin

REM TODO: Get rid of /EHsc once the glTF2 parser is compiled separately

SET EXECUTABLE=kai.exe
SET COMPILER_FLAGS=/nologo /std:c++17 /Od /MTd /Zi /Gm- /EHa- /EHsc /FC /W4 /wd4201 /Fe:%EXECUTABLE%
SET DEFINES=/DKAI_PLATFORM_WIN32 /DKAI_DEBUG /DDEBUG /D_DEBUG /DUNICODE /D_UNICODE
SET LINKER_FLAGS=/INCREMENTAL:NO /SUBSYSTEM:WINDOWS
SET LIBRARIES=kernel32.lib user32.lib ole32.lib d3d11.lib dxgi.lib d3dcompiler.lib

pushd bin
cl %DEFINES% %COMPILER_FLAGS% ..\src\platform\win32\win32_kai.cpp %LIBRARIES% /link %LINKER_FLAGS%
copy /b /y %EXECUTABLE% ..\
popd
