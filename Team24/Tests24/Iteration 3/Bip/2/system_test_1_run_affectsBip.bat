@echo off
title Running system acceptance test case for system_test_1_affectsbip

..\..\..\..\Code24\Debug\AutoTester.exe system_test_1_source.txt system_test_1_queries_affectsbip.txt out-system_test_1_affectsbip_test.xml

findstr "failed" out-system_test_1_affectsbip_test.xml
findstr "crash" out-system_test_1_affectsbip_test.xml
findstr "exception" out-system_test_1_affectsbip_test.xml
findstr "timeout" out-system_test_1_affectsbip_test.xml

echo The End!
pause
exit
