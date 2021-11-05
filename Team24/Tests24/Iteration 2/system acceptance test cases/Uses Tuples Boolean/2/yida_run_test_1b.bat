@echo off
title Running yida system acceptance test case!

..\..\..\..\..\Code24\Release\AutoTester.exe yida_test_source1.txt yida_test_query1b.txt out-test_1b.xml

findstr "failed" out-test_1b.xml
findstr "crash" out-test_1b.xml
findstr "missing" out-test_1b.xml
findstr "additional" out-test_1b.xml

echo The End!
pause
exit