@echo off
title Running calls system acceptance test case!

..\..\..\..\Code24\Debug\AutoTester.exe source.txt nextBip_BipT_queries.txt out-test_nextBip_BipT.xml


findstr "failed" out-test_nextBip_BipT.xml
findstr "crash" out-test_nextBip_BipT.xml
findstr "exception" out-test_nextBip_BipT.xml
findstr "timeout" out-test_nextBip_BipT.xml

echo The End!
pause
exit
