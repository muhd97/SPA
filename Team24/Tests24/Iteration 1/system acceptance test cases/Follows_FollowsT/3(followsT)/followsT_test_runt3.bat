@echo off
title Running follows-FollowsT system acceptance test case!

..\..\..\..\..\Code24\Release\AutoTester.exe followsT_test_source3.txt followsT_test_query3.txt output3.xml

findstr "failed" output3.xml
findstr "exception" output3.xml
findstr "crash" output3.xml

echo The End!
pause
exit