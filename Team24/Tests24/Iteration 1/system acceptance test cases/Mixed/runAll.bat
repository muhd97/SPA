@echo off
title Running All Iteration 1 System Test Cases

for /f %%f in ('dir /ad /b') do (
    echo %cd%\%%f
    pushd %cd%\%%f
    for %%I in (*.bat) do (
        start /wait %%I
        del *.xml
    )
    popd
)


echo The End!
pause
exit