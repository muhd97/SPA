@echo off
title Running follows-FollowsT system acceptance test case!

..\..\..\..\..\Code24\Release\AutoTester.exe follows_test_source2.txt follows_test_query2.txt output2.xml

findstr "failed" output2.xml
findstr "exception" output2.xml
findstr "crash" output2.xml

echo The End!
pause
exit