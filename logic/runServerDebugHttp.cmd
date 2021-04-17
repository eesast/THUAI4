@echo off
.\Logic.Server\bin\Debug\netcoreapp3.1\Logic.Server.exe --port=20000 --teamCount=1 --playerCount=1 -k="haha" -g 5 || pause
echo Server end!
pause
