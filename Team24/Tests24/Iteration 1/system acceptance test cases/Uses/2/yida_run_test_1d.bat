@echo off
title Running yida system acceptance test case!

..\..\..\..\..\Code24\Debug\AutoTester.exe yida_test_source1.txt yida_test_query1d.txt out-test_1d.xml

findstr "failed" out-test_1d.xml
findstr "crash" out-test_1d.xml
findstr "exception" out-test_1d.xml

echo The End!
pause
exit