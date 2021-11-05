@echo off
title Running demo system acceptance test case 3!

..\..\Code24\Release\AutoTester.exe 3_source.txt 3_queries.txt 3_out.xml

findstr "failed" 3_out.xml
findstr "exception" 3_out.xml
findstr "crash" 3_out.xml
findstr "timeout" 3_out.xml

echo The End!
pause
exit