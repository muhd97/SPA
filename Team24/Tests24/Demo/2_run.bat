@echo off
title Running system acceptance test case 1!

..\..\Code24\Debug\AutoTester.exe 2_source.txt 2_queries.txt 2_out.xml

findstr "failed" out-first_test.xml
findstr "exception" out-first_test.xml
findstr "crash" out-first_test.xml
findstr "timeout" out-first_test.xml

echo The End!
pause
exit