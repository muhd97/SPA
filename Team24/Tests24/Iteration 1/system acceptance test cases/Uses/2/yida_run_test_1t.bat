@echo off
title Running yida system acceptance test case!

..\..\..\..\..\Code24\Debug\AutoTester.exe yida_test_source1.txt yida_test_query1t.txt out-test_1t.xml

findstr "failed" out-test_1t.xml

echo The End!
pause
exit