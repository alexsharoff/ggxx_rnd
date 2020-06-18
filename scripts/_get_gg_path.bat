set "gg_path=%~dp0\..\.build\ggxxacpr\bin\gg.exe"
if not exist "%gg_path%" (
    >&2 echo %gg_path% does not exist. Run build.bat first?
    pause
    exit /b 1
)
