@ECHO OFF
SET PATH=%CD%\..\libraries\opencv330\x64\vc14\bin;%PATH%
SET PATH=%CD%\..\libraries\msvcrt\vc14\x64\;%PATH%
SET PATH=%CD%\..\libraries\api-ms-win\x64\;%PATH%
SET PATH=%CD%\..\libraries\qt\bin\;%PATH%
SET PATH=%CD%\..\libraries\vtk8\x64\Release\bin\;%PATH%
%~n0.exe