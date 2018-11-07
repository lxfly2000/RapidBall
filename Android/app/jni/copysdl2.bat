@echo off
::请到www.libsdl.org下载SDL2及相应扩展库并将它们解压至一个空的目录，将该目录的路径填写至下方。
set sdl2_download_path=D:\Yueyu\Codes
::                     ~~~~~~~~~~~~~~更改为SDL2和扩展所在目录

if not exist "%sdl2_download_path%" (
	echo 无法找到%sdl2_download_path%，请用记事本打开这个文件并修改SDL2所在的路径。
	pause
)
for /f "delims=- tokens=1,2*" %%i in ('dir/b/a:d "%sdl2_download_path%\SDL2*"') do (
	xcopy /e /y "%sdl2_download_path%\%%i-%%j" SDL2\%%i\
	xcopy /e /y SDL2\%%i\include SDL2\include\
)
echo include $(call all-subdir-makefiles)>SDL2\Android.mk
