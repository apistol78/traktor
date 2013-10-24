@echo off

call %~dp0config.bat

start build\pnacl\"Traktor PNaCl.sln"
