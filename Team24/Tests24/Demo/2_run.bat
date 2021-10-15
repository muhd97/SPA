@echo off
title Running demo system acceptance test case 2!

..\..\Code24\Debug\AutoTester.exe 2_source.txt 2_queries.txt 2_out.xml

findstr "failed" 2_out.xml
findstr "exception" 2_out.xml
findstr "crash" 2_out.xml
findstr "timeout" 2_out.xml

echo The End!
pause
exit