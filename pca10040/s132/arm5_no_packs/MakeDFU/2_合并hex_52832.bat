mergehex.exe -m s132_nrf52_6.1.1_softdevice.hex ../_build/ble_app_VAQSO_V5.hex -o sd_and_app.hex  
mergehex.exe -m sd_and_app.hex  ../../../../../../dfu/secure_bootloader_VAQSO_V5/pca10040_ble/arm5_no_packs/_build/dfu_bootloader_VAQSO_V5.hex  -o sd_and_app_and_boot.hex
mergehex.exe -m sd_and_app_and_boot.hex DFU_Settings.hex -o sd_app_boot_setting_complete_VAQSO_V5.hex
exit 0