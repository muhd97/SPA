@echo off
title Running follows-FollowsT system acceptance test case!

..\..\..\..\..\Code24\Release\AutoTester.exe follows_followsT_source.txt follows_followsT_query.txt out-follows_followsT_test.xml

echo The End!
pause
exit