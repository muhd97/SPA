# Team 24 
### Iteration 2

## Target Environment

Item | Version
-|-
OS | Windows 10
Toolchain | Microsoft Visual Studio Enterprise 2019 Version 16.11.0
C++ Standard | C++17

### Additional Build Instructions

1. Open StartupSPASolution.sln in Visual Studio Enterprise 2019 Version 16.11.0
2. Right click on the project AutoTester and select Build
3. Run the AutoTester from the command prompt by navigating to the AutoTester.exe file and providing three arguments:
    - filename containing the SIMPLE source program
    - filename containing the queries
    - output xmlfile to store the test results
4. To view the results of the test, open the output .xml file in NotePad or Mozilla Firefox.

For example:
> AutoTester.exe ..\Tests24\first_source.txt ..\Tests24\first_queries.txt ..\Tests24\out.xml

