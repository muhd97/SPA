@echo off
title Running system acceptance test case for system_test_1_nextbip!

..\..\..\..\Code24\Debug\AutoTester.exe system_test_1_source.txt system_test_1_queries_nextbip.txt out-system_test_1_nextbip_test.xml

findstr "failed" out-system_test_1_nextbip_test.xml
findstr "crash" out-system_test_1_nextbip_test.xml
findstr "exception" out-system_test_1_nextbip_test.xml
findstr "timeout" out-system_test_1_nextbip_test.xml

echo The End!
pause
exit
