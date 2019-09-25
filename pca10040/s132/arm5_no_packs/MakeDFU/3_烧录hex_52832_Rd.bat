::nrfjprog.exe --family NRF52  --recover
nrfjprog.exe --family NRF52  --eraseall
nrfjprog.exe --family NRF52  --program MBT009_sd_app_boot_setting_complete.hex 
nrfjprog.exe --family NRF52  --verify MBT009_sd_app_boot_setting_complete.hex
::nrfjprog.exe --family NRF52  --rbp all
nrfjprog.exe  --reset
pause
