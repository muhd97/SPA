@echo off
title Running yida system acceptance test case!

..\..\..\..\..\Code24\Debug\AutoTester.exe yida_test_source1.txt yida_test_query1TEST.txt test_query1TEST.xml

findstr "failed" test_query1TEST.xml
findstr "crash" test_query1TEST.xml
findstr "exception" test_query1TEST.xml

echo The End!
pause
exit