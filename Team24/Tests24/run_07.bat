@echo off
title Running sample system acceptance test case!

..\Code24\Release\AutoTester.exe 07_1_mixed_second_source.txt 07_1_mixed_second_queries.txt sample_out.xml

findstr "/failed" sample_out.xml
findstr "/exception" sample_out.xml
findstr "/crash" sample_out.xml
findstr "timeout" sample_out.xml

echo The End!
pause
exit