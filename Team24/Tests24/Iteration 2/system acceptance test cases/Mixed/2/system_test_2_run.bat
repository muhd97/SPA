@echo off
title Running system acceptance test case for system_test_2!

..\..\..\..\..\Code24\Debug\AutoTester.exe system_test_2_source.txt system_test_2_queries.txt out-system_test_2.xml

findstr "failed" out-system_test_1.xml
findstr "crash" out-system_test_1.xml
findstr "exception" out-system_test_1.xml
findstr "timeout" out-system_test_1.xml

echo The End!
pause
exit
