@echo off
title Running yida system acceptance test case!

..\..\..\..\..\Code24\Release\AutoTester.exe yida_test_source2.txt yida_test_query2b.txt out-test_2b.xml

findstr "failed" out-test_2b.xml
findstr "crash" out-test_2b.xml
findstr "except" out-test_2b.xml


echo The End!
pause
exit