::del *VAQSO_V5*.zip /s
::������%time:~0,2%%time:~3,2%%time:~6,2%ʱ�и����⣬ �������TIME ��00~09���ʱ�򣬵�����ʾ����0 ����00����TIME 0~2 �ͱ�����,https://blog.csdn.net/icanlove/article/details/42640329
::nrfutil pkg generate --application ../_build/ble_app_VAQSO_V5.hex --application-version 0xFF --hw-version 52 --sd-req 0xb7 --key-file key.pem appDfuPack_VAQSO_V5_%Date:~0,4%%Date:~5,2%%Date:~8,2%_%time:~0,2%%time:~3,2%%time:~6,2%%.zip 
for /f "tokens=1,2 delims=:" %%i in ('time/t') do set t=%%i%%j
nrfutil pkg generate --application ../_build/ble_app_VAQSO_V5.hex --application-version 0xFF --hw-version 52 --sd-req 0xb7 --key-file key.pem AppDfuVAQSO_%Date:~0,4%%Date:~5,2%%Date:~8,2%_%t:~0,4%%time:~6,2%.zip 
pause