@echo off

rem XmlTransform -r xtscripts/remove-entityinstance.js ../data/Source/*.xdi
XmlTransform -r xtscripts/update-physics.js ../data/Source/*.xdi
