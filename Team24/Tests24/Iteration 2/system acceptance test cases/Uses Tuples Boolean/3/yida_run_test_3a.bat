@echo off
title Running yida system acceptance test case!

..\..\..\..\..\Code24\Release\AutoTester.exe yida_test_source3.txt yida_test_query3a.txt out-test_3a.xml

findstr "failed" out-test_3a.xml
findstr "crash" out-test_3a.xml
findstr "except" out-test_3a.xml


echo The End!
pause
exit