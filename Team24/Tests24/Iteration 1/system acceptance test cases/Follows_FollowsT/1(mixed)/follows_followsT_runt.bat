@echo off
title Running follows-FollowsT system acceptance test case!

..\..\..\..\..\Code24\Debug\AutoTester.exe follows_followsT_source.txt follows_followsT_queryt.txt output.xml

findstr "failed" output.xml
findstr "exception" output.xml
findstr "crash" output.xml

echo The End!
pause
exit