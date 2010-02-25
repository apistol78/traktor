@echo off

call config.bat

if not exist reports ( mkdir reports )

set REPORT_DB=data/temp/Pipeline.db

%TRAKTOR_HOME%/bin/ReportGenerator %TRAKTOR_HOME%/res/reports/Shaders.html > reports/Shaders.html
%TRAKTOR_HOME%/bin/ReportGenerator %TRAKTOR_HOME%/res/reports/Textures.html > reports/Textures.html
%TRAKTOR_HOME%/bin/ReportGenerator %TRAKTOR_HOME%/res/reports/Meshes.html > reports/Meshes.html
