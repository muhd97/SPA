@echo off
title Running first system acceptance test case!

..\..\..\..\..\Code24\Release\AutoTester.exe valid-08-source.txt valid-08-ParentT_queries.txt validT08.xml

findstr "failed" out-first_source.xml
findstr "exception" out-first_source.xml
findstr "crash" out-first_source.xml

echo The End!
pause
exit