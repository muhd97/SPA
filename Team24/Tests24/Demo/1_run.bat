@echo off
title Running system acceptance test case 1!

..\..\Code24\Debug\AutoTester.exe 1_source.txt 1_queries.txt 1_out.xml

findstr "failed" out-first_test.xml
findstr "exception" out-first_test.xml
findstr "crash" out-first_test.xml
findstr "timeout" out-first_test.xml

echo The End!
pause
exit