@echo off
echo Starting local server for GestureTalk...
echo.
echo NOTE: You need to access the site via the localhost URL (e.g., http://127.0.0.1:8080)
echo for the camera to work properly.
echo.
echo Installing/Running http-server...
cd website
npx -y http-server -o
pause
