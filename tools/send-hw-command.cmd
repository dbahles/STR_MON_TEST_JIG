@echo off
powershell -ExecutionPolicy Bypass -File "%~dp0send-hw-command.ps1" %*
