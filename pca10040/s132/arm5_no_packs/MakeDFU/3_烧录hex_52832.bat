nrfjprog.exe --family NRF52  --recover
nrfjprog.exe --family NRF52  --eraseall
nrfjprog.exe --family NRF52  --program sd_app_boot_setting_complete_VAQSO_V5.hex 
nrfjprog.exe --family NRF52  --verify sd_app_boot_setting_complete_VAQSO_V5.hex
::nrfjprog.exe --family NRF52  --rbp all
nrfjprog.exe  --reset
pause
