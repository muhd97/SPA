@echo off
title Running yida system acceptance test case!

..\..\..\..\..\Code24\Debug\AutoTester.exe yida_test_source1.txt yida_test_query1a.txt out-test_1a.xml

findstr "failed" out-test_1a.xml
findstr "crash" out-test_1a.xml
findstr "exception" out-test_1a.xml


echo The End!
pause
exit