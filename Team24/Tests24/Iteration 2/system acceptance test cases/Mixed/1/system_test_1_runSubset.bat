@echo off
title Running system acceptance test case for system_test_1!

..\..\..\..\..\Code24\Debug\AutoTester.exe system_test_1_source.txt system_test_1_queriesSubset.txt out-system_test_1Subset.xml

findstr "failed" out-system_test_1Subset.xml
findstr "crash" out-system_test_1Subset.xml
findstr "missing" out-system_test_1Subset.xml
findstr "additional" out-system_test_1Subset.xml

echo The End!
pause
exit