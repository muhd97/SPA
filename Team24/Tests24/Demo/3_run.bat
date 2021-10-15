@echo off
title Running system acceptance test case 1!

..\..\Code24\Debug\AutoTester.exe 3_source.txt 3_queries.txt 3_out.xml

findstr "failed" out-first_test.xml
findstr "exception" out-first_test.xml
findstr "crash" out-first_test.xml
findstr "timeout" out-first_test.xml

echo The End!
pause
exit