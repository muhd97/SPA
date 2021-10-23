@echo off
title Running yida system acceptance test case!

..\..\..\..\..\Code24\Debug\AutoTester.exe yida_test_source1empty.txt yida_test_query1empty.txt out-test_1empty.xml


findstr "failed" out-test_1empty.xml
findstr "crash" out-test_1empty.xml
findstr "missing" out-test_1empty.xml
findstr "additional" out-test_1empty.xml

echo The End!
pause
exit