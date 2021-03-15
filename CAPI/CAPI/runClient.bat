@ECHO OFF

START CMD /K Release\CAPI.exe -P 7777 -p 0 -t 0 -j 0 

START CMD /K Release\CAPI.exe -P 7777 -p 0 -t 1 -j 0 

START CMD /K Release\CAPI.exe -P 7777 -p 1 -t 0 -j 1 

START CMD /K Release\CAPI.exe -P 7777 -p 1 -t 1 -j 1 

START CMD /K Release\CAPI.exe -P 7777 -p 2 -t 0 -j 2 

START CMD /K Release\CAPI.exe -P 7777 -p 2 -t 1 -j 2 

START CMD /K Release\CAPI.exe -P 7777 -p 3 -t 0 -j 3 

START CMD /K Release\CAPI.exe -P 7777 -p 3 -t 1 -j 3 
