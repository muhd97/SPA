@echo off
title Running yida system acceptance test case!

..\..\..\..\..\Code24\Release\AutoTester.exe yida_test_source2.txt yida_test_query2a.txt out-test_2a.xml

findstr "failed" out-test_2a.xml
findstr "crash" out-test_2a.xml
findstr "except" out-test_2a.xml

echo The End!
pause
exit