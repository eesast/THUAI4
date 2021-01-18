@echo off
start "servertest" cmd /c call runServertest.bat
start "agent" cmd /c call runAgent.bat
start "clienttest" cmd /c call runClienttest.bat