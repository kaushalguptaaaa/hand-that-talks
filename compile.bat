@echo off
setlocal

echo ================================================================
echo NOTE: DISCONTINUED / LEGACY C++ BUILD
echo The primary demo is now WEB-BASED.
echo Please run "run_demo.bat" instead for the easiest experience!
echo ================================================================
echo.
timeout /t 5

:: CONFIGURATION - USER PLEASE UPDATE THIS PATH
set "OPENCV_DIR=C:\opencv\build"

:: Check if path exists
if not exist "%OPENCV_DIR%" (
   echo [ERROR] OpenCV directory not found at: %OPENCV_DIR%
   echo Please edit compile.bat and set OPENCV_DIR to your OpenCV installation path.
   echo Expected structure: %OPENCV_DIR%\include and %OPENCV_DIR%\x64\mingw\lib
   pause
   exit /b 1
)

:: Attempt to auto-detect version from lib files (simple heuristic)
:: Looking for libopencv_coreXXX.dll.a
pushd "%OPENCV_DIR%\x64\mingw\lib"
for %%f in (libopencv_core*.dll.a) do set "LIB_NAME=%%f"
popd

if "%LIB_NAME%"=="" (
    echo [ERROR] Could not find OpenCV libraries in %OPENCV_DIR%\x64\mingw\lib
    pause
    exit /b 1
)

:: Extract version suffix (e.g., from libopencv_core452.dll.a -> 452)
set "VERSION_SUFFIX=%LIB_NAME:libopencv_core=%"
set "VERSION_SUFFIX=%VERSION_SUFFIX:.dll.a=%"

echo Found OpenCV Version Suffix: %VERSION_SUFFIX%

:: Compilation Command
echo Compiling...
g++ -std=c++11 ^
    -I"%OPENCV_DIR%\include" ^
    -L"%OPENCV_DIR%\x64\mingw\lib" ^
    src/main.cpp src/GestureRecognizer.cpp ^
    -o GestureTalk.exe ^
    -lopencv_core%VERSION_SUFFIX% ^
    -lopencv_highgui%VERSION_SUFFIX% ^
    -lopencv_imgproc%VERSION_SUFFIX% ^
    -lopencv_videoio%VERSION_SUFFIX% ^
    -lopencv_imgcodecs%VERSION_SUFFIX%

if %errorlevel% neq 0 (
    echo [ERROR] Compilation failed.
    pause
    exit /b 1
)

echo [SUCCESS] Build complete. Run GestureTalk.exe to start.
pause
