@ECHO OFF

set stdin=
set has_input=false
set nl=^& echo.

@echo off

set args=
:start
    if [%1] == [] goto main
    if "%1" == "-" set has_input=true
    if "%1" == "--stdin" set has_input=true
    set args=%args%%1
    shift
    goto start

:has_stdin
    setlocal EnableDelayedExpansion
    for /F "tokens=*" %%a in ('findstr /n $') do (
        set "line=%%a"
        set "line=!line:*:=!"
        set stdin=!stdin!!line!!nl!
    )

    goto exec

:main
    if %has_input% == true goto has_stdin

:exec
    ECHO %stdin% | python %~dp0\mapper.py %*