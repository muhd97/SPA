@echo off
title Running calls system acceptance test case!

..\..\..\..\..\Code24\Debug\AutoTester.exe source.txt nextBip_queries.txt out-test_nextBip.xml


findstr "failed" out-test_nextBip.xml
findstr "crash" out-test_nextBip.xml
findstr "missing" out-test_nextBip.xml
findstr "additional" out-test_nextBip.xml

echo The End!
pause
exit