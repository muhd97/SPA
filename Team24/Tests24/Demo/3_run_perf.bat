@echo off
title Running demo system acceptance test case 3!

..\..\Code24\Debug\AutoTester.exe 3_source.txt 3_queries_perf.txt 3_out_perf.xml

findstr "failed" 3_out_perf.xml
findstr "exception" 3_out_perf.xml
findstr "crash" 3_out_perf.xml
findstr "timeout" 3_out_perf.xml

echo The End!
pause
exit