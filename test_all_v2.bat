@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion

echo ========================================
echo Tinc Complete Interface Test Tool
echo ========================================

set SERVER_URL=http://localhost:8081
set BASE_PATH=/XVntQFJCjc.php

echo.
echo ========== 1. Test Login Interface ==========
echo.

set LOGIN_URL=%SERVER_URL%%BASE_PATH%/myadmin/node/api

echo Request URL: %LOGIN_URL%
echo.

echo {"sid":"test_user_001","password":"test_password_123"} > login_data.json

curl -X POST "%LOGIN_URL%" -H "Content-Type: application/json" -d @login_data.json -w "\nHTTP Status: %%{http_code}\n"

del login_data.json

echo.
echo ========================================
echo.

echo ========== 2. Test Download Config Interface ==========
echo.

set DOWNLOAD_URL=%SERVER_URL%%BASE_PATH%/coreplugs/coreplugs/api

echo Request URL: %DOWNLOAD_URL%
echo.

echo {"sid":"test_user_002","token":"test_token_123456"} > download_data.json

curl -X POST "%DOWNLOAD_URL%" -H "Content-Type: application/json" -d @download_data.json -o config.zip -w "\nHTTP Status: %%{http_code}\n"

del download_data.json

if exist config.zip (
    echo.
    echo ZIP file downloaded: config.zip
    echo File size:
    dir config.zip | find "config.zip"
    echo.
)

echo.
echo ========================================
echo.

echo ========== 3. Test Upload Key Interface ==========
echo.

set UPLOAD_URL=%SERVER_URL%%BASE_PATH%/coreplugs/Clientinterface/exchangeFile

echo Request URL: %UPLOAD_URL%
echo.

echo {"sid":"test_user_003","token":"test_token_789012","content":"-----BEGIN PUBLIC KEY-----\nMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAwK5Z7X9j2P8X3Y7R6T1S5\n-----END PUBLIC KEY-----\n","action":"add"} > upload_data.json

curl -X POST "%UPLOAD_URL%" -H "Content-Type: application/json" -d @upload_data.json -w "\nHTTP Status: %%{http_code}\n"

del upload_data.json

echo.
echo ========================================
echo.

echo ========== 4. Test Edit Interface ==========
echo.

set EDIT_URL=%SERVER_URL%%BASE_PATH%/promin/Api/editadd_info

echo Request URL: %EDIT_URL%
echo.

echo {"type":"add","result":"success","ids":"123","details":"Node test_user completed connection"} > edit_data.json

curl -X POST "%EDIT_URL%" -H "Content-Type: application/json" -d @edit_data.json -w "\nHTTP Status: %%{http_code}\n"

del edit_data.json

echo.
echo ========================================
echo.
echo All tests completed!
echo.

pause
