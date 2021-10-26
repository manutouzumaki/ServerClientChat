@ECHO OFF

IF NOT EXIST ..\build mkdir ..\build

pushd ..\build
cl -nologo ..\code\main_window_app.cpp User32.lib Gdi32.lib Kernel32.lib Winmm.lib Ws2_32.lib
popd
