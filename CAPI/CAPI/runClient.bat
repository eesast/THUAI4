@ECHO OFF

START CMD /K x64\Release\CAPI.exe -P 7777 -p 0 -t 0 

START CMD /K x64\Release\CAPI.exe -P 7777 -p 0 -t 1 

START CMD /K x64\Release\CAPI.exe -P 7777 -p 1 -t 0 

START CMD /K x64\Release\CAPI.exe -P 7777 -p 1 -t 1 

START CMD /K x64\Release\CAPI.exe -P 7777 -p 2 -t 0 

START CMD /K x64\Release\CAPI.exe -P 7777 -p 2 -t 1 

START CMD /K x64\Release\CAPI.exe -P 7777 -p 3 -t 0 

START CMD /K x64\Release\CAPI.exe -P 7777 -p 3 -t 1 
