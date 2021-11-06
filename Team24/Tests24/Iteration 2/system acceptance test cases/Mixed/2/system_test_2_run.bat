@echo off
title Running system acceptance test case for system_test_2!

..\..\..\..\..\Code24\Release\AutoTester.exe system_test_2_source.txt system_test_2_queries.txt out-system_test_2.xml

findstr "fail" out-system_test_2.xml
findstr "crash" out-system_test_2.xml
findstr "exception" out-system_test_2.xml
findstr "timeout" out-system_test_2.xml

echo The End!
pause
exit
