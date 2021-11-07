@echo off
title Running system acceptance test case for affectsBipT_test!

..\..\..\..\Code24\Debug\AutoTester.exe affectsBipT_source.txt affectsBipT_queries.txt out-affectsBipT_test.xml

findstr "failed" out-affectsBipT_test.xml
findstr "crash" out-affectsBipT_test.xml
findstr "exception" out-affectsBipT_test.xml
findstr "timeout" out-affectsBipT_test.xml

echo The End!
pause
exit
