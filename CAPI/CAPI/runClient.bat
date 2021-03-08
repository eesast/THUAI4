@ECHO OFF

START CMD /K Release\CAPI.exe -P 7777 -p 0 -t 0 -j 0 
::@ping 127.0.0.1 -n 2 > NUL
START CMD /K Release\CAPI.exe -P 7777 -p 0 -t 1 -j 0 
::@ping 127.0.0.1 -n 2 > NUL
START CMD /K Release\CAPI.exe -P 7777 -p 1 -t 0 -j 1 
::@ping 127.0.0.1 -n 2 > NUL
START CMD /K Release\CAPI.exe -P 7777 -p 1 -t 1 -j 1 
::@ping 127.0.0.1 -n 2 > NUL
START CMD /K Release\CAPI.exe -P 7777 -p 2 -t 0 -j 2 
::@ping 127.0.0.1 -n 2 > NUL
START CMD /K Release\CAPI.exe -P 7777 -p 2 -t 1 -j 2 
::@ping 127.0.0.1 -n 2 > NUL
START CMD /K Release\CAPI.exe -P 7777 -p 3 -t 0 -j 3 
::@ping 127.0.0.1 -n 2 > NUL
START CMD /K Release\CAPI.exe -P 7777 -p 3 -t 1 -j 3 
