@echo off
title Running calls system acceptance test case!

..\..\..\..\Code24\Debug\AutoTester.exe source.txt affectsBip_BipT_queries.txt out-test_affectsBip_BipT.xml


findstr "failed" out-test_affectsBip_BipT.xml
findstr "crash" out-test_affectsBip_BipT.xml
findstr "exception" out-test_affectsBip_BipT.xml
findstr "timeout" out-test_affectsBip_BipT.xml

echo The End!
pause
exit
