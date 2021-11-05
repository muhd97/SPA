@echo off
title Running calls system acceptance test case!

..\..\..\..\..\Code24\Release\AutoTester.exe 1_source.txt 1_query.txt out-test_calls1.xml


findstr "failed" out-test_calls1.xml
findstr "crash" out-test_calls1.xml
findstr "missing" out-test_calls1.xml
findstr "additional" out-test_calls1.xml

echo The End!
pause
exit