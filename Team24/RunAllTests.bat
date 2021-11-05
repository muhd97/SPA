@echo off
title Running all test cases!

DEL /Q .\Tests24\Outputs\*.xml
python RunAllTests.py

echo The End!
pause
exit