del *VAQSO_V5*.zip /s
nrfutil pkg generate --application ../_build/ble_app_VAQSO_V5.hex --application-version 0xFF --hw-version 52 --sd-req 0xb7 --key-file key.pem appDfuPack_VAQSO_V5_%Date:~0,4%%Date:~5,2%%Date:~8,2%_%time:~0,2%%time:~3,2%%time:~6,2%%.zip 
pause