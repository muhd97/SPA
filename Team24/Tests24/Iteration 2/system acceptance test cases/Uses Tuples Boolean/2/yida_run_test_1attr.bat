@echo off
title Running yida system acceptance test case!

..\..\..\..\..\Code24\Debug\AutoTester.exe yida_test_source1attr.txt yida_test_query1attr.txt out-test_1attr.xml

findstr "failed" out-test_1attr.xml
findstr "crash" out-test_1attr.xml
findstr "exception" out-test_1attr.xml

echo The End!
pause
exit