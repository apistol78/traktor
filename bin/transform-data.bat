@echo off

rem XmlTransform -r xtscripts/remove-entityinstance.js ../data/Source/*.xdi
rem XmlTransform -r xtscripts/update-physics.js ../data/Source/*.xdi
rem XmlTransform -r xtscripts/update-theater.js ../data/Source/*.xdi

XmlTransform -r xtscripts/update-input.js ../samples/InputTest/data/*.xml
