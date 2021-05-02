@echo off
copy /Y x64\Release\PlayBackPlayerResources.dll PlayBackPlayerDll\*
copy /Y x64\Release\PlayBackPlayerResources.dll PlayBackPlayer\bin\Release\netcoreapp3.1\*
copy /Y x64\Release\PlayBackPlayerResources.dll PlayBackPlayer\bin\Debug\netcoreapp3.1\*
copy /Y x64\Release\PlayBackPlayerResources.dll ..\logic\publish\PlayBackPlayer\*
pause
