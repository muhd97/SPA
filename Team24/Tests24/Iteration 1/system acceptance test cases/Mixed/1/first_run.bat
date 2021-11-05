@echo off
title Running first system acceptance test case!

..\..\..\..\..\Code24\Release\AutoTester.exe first_source.txt first_queries.txt out-first_test.xml

findstr "failed" out-first_test.xml
findstr "exception" out-first_test.xml
findstr "crash" out-first_test.xml

echo The End!
pause
exit