@echo off
title Running calls system acceptance test case!

..\..\..\..\..\Code24\Debug\AutoTester.exe source.txt affectsBip_queries.txt out-test_affectsBip.xml


findstr "failed" out-test_affectsBip.xml
findstr "crash" out-test_affectsBip.xml
findstr "missing" out-test_affectsBip.xml
findstr "additional" out-test_affectsBip.xml

echo The End!
pause
exit