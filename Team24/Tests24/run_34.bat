@echo off
title Running sample system acceptance test case!

..\Code24\Release\AutoTester.exe 34_3_system_test_1_source.txt 34_3_system_test_1_affectsbip_queries.txt sample_out.xml

findstr "/failed" sample_out.xml
findstr "/exception" sample_out.xml
findstr "/crash" sample_out.xml
findstr "timeout" sample_out.xml

echo The End!
pause
exit