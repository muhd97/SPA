@echo off
title Running first system acceptance test case!

..\..\..\..\..\Code24\Release\AutoTester.exe valid-08-source.txt valid-08-Parent_queries.txt valid08.xml

findstr "failed" valid08.xml
findstr "exception" valid08.xml
findstr "crash" valid08.xml

echo The End!
pause
exit