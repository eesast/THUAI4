@echo off
setlocal enabledelayedexpansion
set /a id=0
for /L %%i in (1,1,4) do  (
start .\clienttest\bin\Debug\netcoreapp3.1\clienttest.exe 0 !id! 7777
set /a id+=1
start .\clienttest\bin\Debug\netcoreapp3.1\clienttest.exe 1 !id! 7777
set /a id+=1
)