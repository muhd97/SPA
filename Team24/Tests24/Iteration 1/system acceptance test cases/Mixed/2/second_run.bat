@echo off
title Running second system acceptance test case!

..\..\..\..\..\Code24\Release\AutoTester.exe second_source.txt second_queries.txt out-second_test.xml

findstr "failed" out-second_test.xml
findstr "crash" out-second_test.xml
findstr "exception" out-second_test.xml

echo The End!
pause
exit