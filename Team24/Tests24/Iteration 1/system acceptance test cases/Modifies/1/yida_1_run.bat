@echo off
title Running yida system acceptance test case!

..\..\..\..\..\Code24\Release\AutoTester.exe yida_test_source1.txt yida_modifies_test_query1.txt out-yida_1.xml

findstr "failed" out-yida_1.xml
findstr "exception" out-yida_1.xml
findstr "crash" out-yida_1.xml

echo The End!
pause
exit