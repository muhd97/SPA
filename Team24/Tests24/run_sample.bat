@echo off
title Running sample system acceptance test case!

..\Code24\Debug\AutoTester.exe sample_source.txt sample_queries.txt sample_out.xml

findstr "/failed" out-first_test.xml
findstr "/exception" out-first_test.xml
findstr "/crash" out-first_test.xml
findstr "timeout" out-first_test.xml

echo The End!
pause
exit