@echo off
title Running demo system acceptance test case 1!

..\..\Code24\Debug\AutoTester.exe 1_source.txt 1_queries.txt 1_out.xml

findstr "failed" 1_out.xml
findstr "exception" 1_out.xml
findstr "crash" 1_out.xml
findstr "timeout" 1_out.xml

echo The End!
pause
exit