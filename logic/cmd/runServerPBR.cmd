@echo off
..\.\Logic.Server\bin\Release\netcoreapp3.1\Logic.Server.exe --port=20000 --teamCount=2 --playerCount=4 -g 600 --fileName=private\res\5z --playBack --playBackSpeed=4.0 --resultOnly
pause
