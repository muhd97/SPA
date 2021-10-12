@echo off
title Running yida system acceptance test case!

..\..\..\..\..\Code24\Debug\AutoTester.exe yida_test_source1.txt yida_test_query1c.txt out-test_1c.xml

findstr "failed" out-test_1c.xml
findstr "crash" out-test_1c.xml
findstr "exception" out-test_1c.xml


echo The End!
pause
exit