@echo off
::�뵽www.libsdl.org����SDL2����Ӧ��չ�Ⲣ�����ǽ�ѹ��һ���յ�Ŀ¼������Ŀ¼��·����д���·���
set sdl2_download_path=D:\Yueyu\Codes
::                     ~~~~~~~~~~~~~~����ΪSDL2����չ����Ŀ¼

if not exist "%sdl2_download_path%" (
	echo �޷��ҵ�%sdl2_download_path%�����ü��±�������ļ����޸�SDL2���ڵ�·����
	pause
)
for /f "delims=- tokens=1,2*" %%i in ('dir/b/a:d "%sdl2_download_path%\SDL2*"') do (
	xcopy /e /y "%sdl2_download_path%\%%i-%%j" SDL2\%%i\
	xcopy /e /y SDL2\%%i\include SDL2\include\
)
echo include $(call all-subdir-makefiles)>SDL2\Android.mk
