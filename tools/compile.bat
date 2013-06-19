@echo off
cd ..
del mp.exe
cd tools
nmake
echo /**********************************************
if not exist ..\mp.exe echo can not create mp.exe,please check your code!
if exist ..\mp.exe echo compile is successful,you can run mp.exe now!
echo ***********************************************/
pause