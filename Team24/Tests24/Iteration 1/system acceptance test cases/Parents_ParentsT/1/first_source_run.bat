@echo off
title Running first system acceptance test case!

..\..\..\..\..\Code24\Release\AutoTester.exe first_source.txt first_queries_parent_only.txt out-first_source.xml

findstr "failed" out-first_source.xml
findstr "exception" out-first_source.xml
findstr "crash" out-first_source.xml

echo The End!
pause
exit