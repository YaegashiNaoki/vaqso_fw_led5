/**
  ******************************************************************************
  * @file    xtcp.c
  * @author  Merle
  * @version V1.0.0
  * @date    9-July-2019
  * @brief   .
  ******************************************************************************
  */
	
/* Includes ------------------------------------------------------------------*/
#include "board.h"


//#define DBG_XTCP
#ifdef DBG_XTCP
#define XTCP_DMSG	RTT_Printf
#define SNPRINTF    snprintf
#else
#define XTCP_DMSG(...)
#define SNPRINTF(...)
#endif

xtcp_packet_t packet_send = {0};



// [0]  [1]        [2]  [3][4]                 [5][...]       [6...]               [7...]         
// 7E   cmd_type   cmd  payload_data_len(U16)  payload_data   data_xor[1~5...]     7F

uint16_t XTCP_MakePackage(uint8_t cmd_type, uint8_t cmd, uint16_t param_data_len, uint8_t const* p_param_data, uint8_t* p_resp_pkg)
{
    if (p_resp_pkg == NULL)
    {
        return 0;
    }
    //xSemaphoreTake(xSemaphore, 1000);//portMAX_DELAY
    
    uint16_t len = 0;
    uint16_t pkg_len = 0;
    uint16_t i;
    uint8_t pkg_xor = 0;
    uint8_t pkg_buf[param_data_len + 6];
    
    // header
    pkg_buf[len++] = 0x7E;

    // cmd type 
    pkg_buf[len++] = cmd_type;
    
    // cmd
    pkg_buf[len++] = cmd;// | 0x80;
    
    // payload data len
    pkg_buf[len++] = param_data_len & 0xFF;
    pkg_buf[len++] = param_data_len >> 8;

    // payload data 
    for (i = 0; i < param_data_len; i++)
    {
        pkg_buf[len++] = p_param_data[i];
    }

    // calc xor_crc, from cmd_type byte to the byte before data_xor(Before escaping)
    for (i = 1; i < len; i++)
    {
        pkg_xor ^= pkg_buf[i];
    }

    // data_xor
    pkg_buf[len++] = pkg_xor;
    
    // end mask
    pkg_buf[len++] = 0x7F;

    // escaping (转义)
    p_resp_pkg[pkg_len++] = 0x7E;

    for (i = 1; i < len - 1; i++)
    {
        switch (pkg_buf[i])
        {
        case 0x7D:
            p_resp_pkg[pkg_len ++] = 0x7D;
            p_resp_pkg[pkg_len ++] = 0x01;
            break;
        case 0x7E:
            p_resp_pkg[pkg_len ++] = 0x7D;
            p_resp_pkg[pkg_len ++] = 0x02;
            break;
        case 0x7F:
            p_resp_pkg[pkg_len ++] = 0x7D;
            p_resp_pkg[pkg_len ++] = 0x03;
            break; 
        default:
            p_resp_pkg[pkg_len ++] = pkg_buf[i];
            break;
        }
    }

    p_resp_pkg[pkg_len ++] = 0x7F;
    
    //xSemaphoreGive(xSemaphore);
    return pkg_len;
}

uint16_t XTCP_MakeErrorCodeRespPkg(uint8_t cmd_type, uint8_t cmd, uint8_t err_code, uint8_t* p_resp_pkg)
{
	uint16_t resp_pkg_len = 0;
    uint8_t packet_resp_err_code = err_code;

    resp_pkg_len = XTCP_MakePackage(cmd_type, cmd, 1, &packet_resp_err_code, p_resp_pkg);
    return resp_pkg_len;
}


uint16_t XTCP_MakeParamDataLenErrorRespPkg(uint16_t param_data_len, uint16_t receive_len_should_be, uint8_t* p_resp_pkg)
{
    XTCP_DMSG("XTCP_CMD_ERROR_PARAM_DATA_INVALID, param_data_len= %d, receive_len_should_be= %d.\r\n",
    
    param_data_len, receive_len_should_be);
	uint16_t resp_pkg_len = 0;
    uint16_t resp_param_data[2];
    resp_param_data[0] = param_data_len;
    resp_param_data[1] = receive_len_should_be;
    resp_pkg_len = XTCP_MakePackage(CMD_ERROR, CMD_ERROR_PARAM_DATA_INVALID, 4, (uint8_t*)&resp_param_data[0], p_resp_pkg);
    return resp_pkg_len;
}


// [0]  [1]        [2]  [3][4]                 [5][...]       [6...]               [7...]         
// 7E   cmd_type   cmd  param_data_len(U16)    param_data     data_xor[1~5...]     7F

uint16_t XTCP_ParsePackage(uint8_t* req_pkg, uint16_t req_pkg_len, uint8_t* resp_pkg, uint16_t max_resp_pkg_len)
{
    uint16_t resp_pkg_len = 0;
    uint16_t i;
    
    uint32_t err_code = 0;
	uint8_t resp_err_code = 0;

    if ((req_pkg[0] != 0x7E) || (req_pkg[req_pkg_len - 1] != 0x7F))
    {
        return 0;
    }

    //escape code (转义) 
    uint8_t num_0x7D = 0;
    
    for (i = 1; i < req_pkg_len - 1; i++)
    {
        if (req_pkg[i + num_0x7D] == 0x7D)
        {
            if (req_pkg[i + num_0x7D + 1] == 0x01)
            {
                req_pkg[i] = 0x7D;
                num_0x7D++;
            }
            else if (req_pkg[i + num_0x7D + 1] == 0x02)
            {
                req_pkg[i] = 0x7E;
                num_0x7D++;
            }
            else if (req_pkg[i + num_0x7D + 1] == 0x03)
            {
                req_pkg[i] = 0x7F;
                num_0x7D++;
            }
            else
            {
                // can not be here.
                XTCP_DMSG("\r\n\r\n[Parse..]error->.buf[n]=0x7D, buf[n+1]=0x%02X.\r\n\r\n",req_pkg[i + num_0x7D + 1] );
                uint8_t resp_param_data[1];
                resp_param_data[0] = req_pkg[i + num_0x7D + 1];
                resp_pkg_len = XTCP_MakePackage(CMD_ERROR, CMD_ERROF_DATA_HAVE_INVALID_0X7D, 1, &resp_param_data[0], resp_pkg);
                return resp_pkg_len;
            }
        }
        else
        {
            req_pkg[i] = req_pkg[i + num_0x7D];
        }
    }

    XTCP_DMSG("[Parse..] 000.req_pkg_len=%d, num_0x7D=%d.\r\n", req_pkg_len, num_0x7D);
    req_pkg_len -= num_0x7D;  // calc the real package len.(计算转义后的实际数据长度)
    req_pkg[req_pkg_len - 1] = 0x7F;
    
    // [0]  [1]        [2]  [3][4]                 [5][...]       [6...]               [7...]         
    // 7E   cmd_type   cmd  param_data_len(U16)    param_data     data_xor[1~5...]     7F
    if (req_pkg_len < 7)
    {
        XTCP_DMSG("[Parse..] 0.rev_err ,pakage_len < 7, len=%d.\r\n", req_pkg_len);
        resp_pkg_len = XTCP_MakePackage(CMD_ERROR, CMD_ERROR_PACKAGE_INVALID, 0, NULL, resp_pkg);
        return resp_pkg_len;
        //return 0;
    }

    // check the sum of xor. (校验和检验)
    uint8_t cal_crc_xor = 0;

    for (int i = 1; i < req_pkg_len - 2; i++)
    {
        cal_crc_xor ^= req_pkg[i];
    }

    uint8_t crc_xor = req_pkg[req_pkg_len - 2];

    if (crc_xor != cal_crc_xor)
    {
        XTCP_DMSG("[Parse..] 1.rev_crc_err ,rev_crc= 0x%02X., calc_crc= 0x%02X.\r\n", crc_xor, cal_crc_xor);
        
        uint8_t resp_param_data[2];
        resp_param_data[0] = crc_xor;
        resp_param_data[1] = cal_crc_xor;
        resp_pkg_len = XTCP_MakePackage(CMD_ERROR, CMD_ERROR_CALC_DATA_XOR, 2, &resp_param_data[0], resp_pkg);
        return resp_pkg_len;
    }

    // Check the data len. (数据长度检验)
    int param_data_len = req_pkg_len - 1 - 2 - 2 - 1 - 1;

    if ((*(uint16_t*)(&req_pkg[3])) != param_data_len)
    {
        XTCP_DMSG("[Parse..] 2.rev_len_err ,rev_len= 0x%04X., calc_len= 0x%04X.\r\n", *(uint16_t*)(&req_pkg[3]), param_data_len);
        
        uint16_t resp_param_data[2];
        resp_param_data[0] = *(uint16_t*)(&req_pkg[3]);
        resp_param_data[1] = param_data_len;
        resp_pkg_len = XTCP_MakePackage(CMD_ERROR, CMD_ERROR_CALC_PARAM_DATA_LEN, 4, (uint8_t*)&resp_param_data[0], resp_pkg);
        return resp_pkg_len;
    }

    // [0]  [1]        [2]  [3][4]                 [5][...]       [6...]               [7...]         
    // 7E   cmd_type   cmd  param_data_len(U16)    param_data     data_xor[1~5...]     7F

    uint8_t cmd_type = req_pkg[1];
    uint8_t cmd = req_pkg[2];
    uint8_t* req_param_data = &req_pkg[5];
    
    // cmd_type
    switch (cmd_type)
    {
        // cmd        
        case CMD_CORE:
            switch (cmd)
            {
                case CMD_CORE_ENTER_DFU_MODE:
                    {
                        XTCP_DMSG("<XTCP> CMD_CORE_ENTER_DFU_MODE.\r\n");
                        
                        uint32_t err_code;
                        err_code = ble_dfu_buttonless_bootloader_start_prepare();
                        
                        if (err_code)
                        {
                            resp_err_code = 0xE0;
                        }
                        resp_pkg_len = XTCP_MakeErrorCodeRespPkg(cmd_type, cmd, resp_err_code, resp_pkg);
                    }
                    break;
                    
                case CMD_CORE_GET_FIRMWARE_VERSION:
                    {
                        XTCP_DMSG("<XTCP> CMD_CORE_GET_FIRMWARE_VERSION.\r\n");
                        
                        uint16_t receive_len_should_be = 0;
                        if (param_data_len != receive_len_should_be)
                        {
                            resp_pkg_len = XTCP_MakeParamDataLenErrorRespPkg(param_data_len, receive_len_should_be, resp_pkg);
                        }
                        else
                        {
                            uint8_t param_data[11];
                            param_data[0] = resp_err_code;
                            // Version
                            param_data[1] = deviceFirmwareVersion[0];
                            param_data[2] = deviceFirmwareVersion[1];
                            param_data[3] = deviceFirmwareVersion[2];
                            // Build Date
                            param_data[4] = deviceFirmwareBuidTime[0];
                            param_data[5] = deviceFirmwareBuidTime[1];
                            param_data[6] = deviceFirmwareBuidTime[2];
                            param_data[7] = deviceFirmwareBuidTime[3];
                            param_data[8] = deviceFirmwareBuidTime[4];
                            param_data[9] = deviceFirmwareBuidTime[5];
                            param_data[10] = deviceFirmwareBuidTime[6];

                            uint16_t param_data_len = sizeof(param_data) / sizeof(uint8_t);
                            
                            resp_pkg_len = XTCP_MakePackage(cmd_type, cmd, param_data_len, (uint8_t const*)&param_data, resp_pkg);
                        }
                    }
                    break;
                
                case CMD_CORE_GET_BATTERY_INFO:
                    {
                        XTCP_DMSG("<XTCP> CMD_CORE_GET_BATTERY_INFO. state= %d, percentage= %d.\r\n",
                                    bat.chargeState,
                                    bat.percentage);
                        
                        uint8_t param_data[4];
                        param_data[0] = resp_err_code;
                        static uint8_t serial_num = 0;
                        param_data[1] = serial_num++;
                        param_data[2] = bat.chargeState;
                        param_data[3] = bat.percentage / 10;
                        uint16_t param_data_len = sizeof(param_data)/ sizeof(uint8_t);
                        
                        resp_pkg_len = XTCP_MakePackage(cmd_type, cmd, param_data_len, (uint8_t const*)&param_data, resp_pkg);
                    }
                    break;
                    
                case CMD_CORE_GET_BLE_DEVICE_UDID:
                    {
                        XTCP_DMSG("<XTCP> CMD_CORE_GET_BLE_DEVICE_UDID.\r\n");
                        
                        uint8_t param_data[9];
                        param_data[0] = resp_err_code;

                        param_data[1] = ((NRF_FICR->DEVICEADDR[0]>> 0) & 0xff);
                        param_data[2] = ((NRF_FICR->DEVICEADDR[0]>> 8) & 0xff);
                        param_data[3] = ((NRF_FICR->DEVICEADDR[0]>>16) & 0xff);
                        param_data[4] = ((NRF_FICR->DEVICEADDR[0]>>24) & 0xff);
                        
                        param_data[5] = ((NRF_FICR->DEVICEADDR[1]>> 0) & 0xff);
                        param_data[6] = ((NRF_FICR->DEVICEADDR[1]>> 8) & 0xff);
                        param_data[7] = ((NRF_FICR->DEVICEADDR[1]>>16) & 0xff);
                        param_data[8] = ((NRF_FICR->DEVICEADDR[1]>>24) & 0xff);
                        uint16_t param_data_len = sizeof(param_data)/ sizeof(uint8_t);
                        
                        resp_pkg_len = XTCP_MakePackage(cmd_type, cmd, param_data_len, (uint8_t const*)&param_data, resp_pkg);
                    
                    }
                    break;
                
                case CMD_CORE_REBOOT:
                    {
                        XTCP_DMSG("<XTCP> CMD_CORE_REBOOT.\r\n");
                        
                        // Here may have some reason cann't let the system reboot right now.
                        if (true)
                        {
                            env.cfg.reboot_status = BOARD_REBOOT_BYTE;
                            ENV_Save();
                            XTCP_DMSG("<XTCP> System In Reboot.\r\n");
                            delay_ms(10);
                            NVIC_SystemReset();
                        }
                        else
                        {
                            resp_err_code = 0xE0;
                            resp_pkg_len = XTCP_MakeErrorCodeRespPkg(cmd_type, cmd, resp_err_code, resp_pkg);
                        }
                    }
                    break;
                    
                default:
                    XTCP_DMSG("<XTCP> CMD_CORE have not the cmd, -> 0x%02X.\r\n", cmd);
                    resp_pkg_len = XTCP_MakePackage(CMD_ERROR, CMD_ERROR_CMD_INVALID, 0, NULL, resp_pkg);
                    break;
            }
            break;
            
        case CMD_AIR_FAN:
            switch (cmd)
            {
                    case CMD_AIR_FAN_SPEED_CONTROL_ALL:
                    {   
                        XTCP_DMSG("<XTCP> CMD_AIR_FAN_SPEED_CONTROL_ALL set.\r\n"); 
                        
                        pkg_air_fan_set_all_t* fan = (pkg_air_fan_set_all_t*)&req_param_data[0];
                        
                        uint16_t receive_len_should_be = sizeof(pkg_air_fan_set_all_t) / sizeof(uint8_t);
                        if (param_data_len != receive_len_should_be)
                        {
                            resp_pkg_len = XTCP_MakeParamDataLenErrorRespPkg(param_data_len, receive_len_should_be, resp_pkg);
                        }
                        else
                        {
                            bool ret = true;
                            
                            for (uint8_t i = 0; i < 5; i++)
                            {
                                ret = FAN_SetSpeed(i, fan->channel[i].speed);
                                if (fan->channel[i].speed == 0)
                                {
                                    LIGHT_SequenceSet(LIGHT_AIR_FAN1_TURN_OFF + i);
                                }
                                else
                                {
                                    LIGHT_SequenceSet(LIGHT_AIR_FAN1_TURN_ON + i);
                                }
                                
                            }
                           
                            if (!ret)
                            {
                                resp_err_code = 0xE0;
                            }
                            resp_pkg_len = XTCP_MakeErrorCodeRespPkg(cmd_type, cmd, resp_err_code, resp_pkg);
                        }            
                    }
                    break;
                    
                case CMD_AIR_FAN_SPEED_CONTROL_ALONE:
                    {   
                        XTCP_DMSG("<XTCP> CMD_AIR_FAN_SPEED_CONTROL_ALONE set.\r\n"); 
                        
                        pkg_air_fan_set_alone_t *fan =  (pkg_air_fan_set_alone_t*)&req_param_data[0];
                        
                        uint16_t receive_len_should_be = sizeof(pkg_air_fan_set_alone_t) / sizeof(uint8_t);
                        if (param_data_len != receive_len_should_be)
                        {
                            resp_pkg_len = XTCP_MakeParamDataLenErrorRespPkg(param_data_len, receive_len_should_be, resp_pkg);
                        }
                        else
                        {
                            bool ret = true;
                            ret = FAN_SetSpeed(fan->channel - 1, fan->speed);
                            if (fan->speed == 0)
                            {
                                LIGHT_SequenceSet(LIGHT_AIR_FAN1_TURN_OFF + fan->channel - 1);
                            }
                            else
                            {
                                LIGHT_SequenceSet(LIGHT_AIR_FAN1_TURN_ON + fan->channel - 1);
                            }
                            
                            if (!ret)
                            {
                                resp_err_code = 0xE0;
                            }
                            resp_pkg_len = XTCP_MakeErrorCodeRespPkg(cmd_type, cmd, resp_err_code, resp_pkg);
                        }            
                    }
                    break;
                    
                default:
                    XTCP_DMSG("<XTCP> CMD_AIR_FAN have not the cmd, -> 0x%02X.\r\n", cmd);
                    resp_pkg_len = XTCP_MakePackage(CMD_ERROR, CMD_ERROR_CMD_INVALID, 0, NULL, resp_pkg);
                    break;
            }
            break;
            
        case CMD_LIGHT:
            switch (cmd)
            {
                case CMD_LIGHT_COLOR_SET_ALONE:
                    {
                        XTCP_DMSG("<XTCP> CMD_LIGHT_SET_COLOR_ALONE set num=%d.\r\n", req_param_data[0]);
                        
                        uint8_t the_rgb_led_set_num = req_param_data[0];
                        
                        uint16_t receive_len_should_be = 1 + the_rgb_led_set_num * sizeof(color_t) / sizeof(uint8_t);
                        if (param_data_len != receive_len_should_be)
                        {
                            resp_pkg_len = XTCP_MakeParamDataLenErrorRespPkg(param_data_len, receive_len_should_be, resp_pkg);
                        }
                        else
                        {
                            uint8_t pixel_num = WS2812B_GetPixelNum();  //15
                            
                            if ((the_rgb_led_set_num >= 1) && (the_rgb_led_set_num <= pixel_num))
                            {
                                LIGHT_SequenceSet(LIGHT_DO_NOTHING); // 

                                color_t* p_color = (color_t*)&req_param_data[1];

                                for (uint8_t i = 0; i < the_rgb_led_set_num; i++)
                                {
                                    WS2812B_SetPixelColor(i, p_color);
                                    p_color++;
                                }
                                WS2812B_Refresh();
                            }
                            else
                            {
                                resp_err_code = 0xE0;
                            }
                            
                            resp_pkg_len = XTCP_MakeErrorCodeRespPkg(cmd_type, cmd, resp_err_code, resp_pkg);         
                        }                 
                    }
                    break;
                
                case CMD_LIGHT_COLOR_FILL_ALL_OR_PART:
                    {
                        XTCP_DMSG("<XTCP> CMD_LIGHT_COLOR_FILL_ALL_OR_PART set.\r\n");  
                        
                        uint16_t receive_len_should_be = sizeof(light_fill_all_or_part_color_t) / sizeof(uint8_t);
                        if (param_data_len != receive_len_should_be)
                        {
                            resp_pkg_len = XTCP_MakeParamDataLenErrorRespPkg(param_data_len, receive_len_should_be, resp_pkg);
                        }
                        else
                        {
                            light_fill_all_or_part_color_t* p_light_set = (light_fill_all_or_part_color_t*)&req_param_data[0];
                            
                            uint8_t pixel_num = WS2812B_GetPixelNum();  //15
                            uint8_t pixel_last_index = p_light_set->first + p_light_set->count; //16
                            if (
                                ((p_light_set->first <= pixel_num) && (p_light_set->first >= 1))&&
                                ((p_light_set->count >= 1) && (pixel_last_index <= (pixel_num + 1)))
                            )
                            {
                                LIGHT_SequenceSet(LIGHT_DO_NOTHING);
                                
                                WS2812B_FillColor(&p_light_set->color, p_light_set->first - 1, p_light_set->count);
                                WS2812B_Refresh();
                            }
                            else
                            {
                                resp_err_code = 0xE0;
                            }
                            
                            resp_pkg_len = XTCP_MakeErrorCodeRespPkg(cmd_type, cmd, resp_err_code, resp_pkg);  
                        }
                    }                    
                    break;
                    
                case CMD_LIGHT_SEQUENCE_SET:
                    {                        
                        XTCP_DMSG("<XTCP> CMD_LIGHT_SEQUENCE_SET set.\r\n");
                        
                        uint16_t receive_len_should_be = sizeof(uint8_t) / sizeof(uint8_t);
                        if (param_data_len != receive_len_should_be)
                        {
                            resp_pkg_len = XTCP_MakeParamDataLenErrorRespPkg(param_data_len, receive_len_should_be, resp_pkg);
                        }
                        else
                        {
                            uint8_t light_sequence_type = req_param_data[0];
                            if (light_sequence_type == 0x00)
                            {
                                LIGHT_SequenceFlush();
                            }
                            else if (light_sequence_type >= 0x01 && light_sequence_type <= 0x05)
                            {
                                light_sequence_type_t type = LIGHT_CARTRIDGE_ECPIRATION_1 + req_param_data[0] - 1;
                                err_code = LIGHT_SequenceSet(type);
                                XTCP_DMSG("<XTCP> LIGHT_SequenceSet type=0x%02X.\r\n", type);
                                if (err_code != NRF_SUCCESS)
                                {
                                    resp_err_code = 0xE1;
                                }
                            }
                            else
                            {
                                resp_err_code = 0xE0;
                            }
                            resp_pkg_len = XTCP_MakeErrorCodeRespPkg(cmd_type, cmd, resp_err_code, resp_pkg);
                        }
                    }
                    break;
                
                default:
                    XTCP_DMSG("<XTCP> CMD_LIGHT have not the cmd, -> 0x%02X.\r\n", cmd);
                    resp_pkg_len = XTCP_MakePackage(CMD_ERROR, CMD_ERROR_CMD_INVALID, 0, NULL, resp_pkg);
                    break;
            }
            break;
            
        case CMD_CARTRIDGE:
            switch (cmd)
            {
                case CMD_CARTRIDGE_GET_STATUS:
                    {
                        XTCP_DMSG("<XTCP> CMD_CARTRIDGE_GET_STATUS.\r\n");
                        
                        uint16_t receive_len_should_be = 0;
                        if (param_data_len != receive_len_should_be)
                        {
                            resp_pkg_len = XTCP_MakeParamDataLenErrorRespPkg(param_data_len, receive_len_should_be, resp_pkg);
                        }
                        else
                        {
                            bool ret;
                            resp_cartridge_get_status_data_t resp_data_cartridge;
                            
                            for (uint8_t i = 0; i < 5; i++)
                            {
                                ret = CARTRIDGE_CheckIn(i);
                                if (ret)
                                {
                                    resp_data_cartridge.channel[i].status = 0x01; // Attach
                                }
                                else
                                {
                                    resp_data_cartridge.channel[i].status = 0x02; // Detach
                                }
                                XTCP_DMSG("<XTCP> cartridge check in channel %d, %s\r\n", 
                                                (i + 1), (ret == true) ? "successful" : "failure");       
                            }
                            
                            uint16_t resp_data_len = sizeof(resp_cartridge_get_status_data_t) / sizeof(uint8_t);
                            
                            resp_pkg_len = XTCP_MakePackage(cmd_type, cmd, resp_data_len, (uint8_t const*)&resp_data_cartridge, resp_pkg);
                        }                            
                    }
                    break;
                    
                case CMD_CARTRIDGE_INFO_WRITE:
                    {
                        XTCP_DMSG("<XTCP> CMD_CARTRIDGE_INFO_WRITE.\r\n");
                        
                        uint16_t receive_len_should_be = 1 + sizeof(cartridge_info_t) / sizeof(uint8_t);
                        if (param_data_len != receive_len_should_be)
                        {
                            resp_pkg_len = XTCP_MakeParamDataLenErrorRespPkg(param_data_len, receive_len_should_be, resp_pkg);
                        }
                        else
                        {
                            bool ret = false;
                            uint8_t cartridge_channel = req_param_data[0];
                            cartridge_info_t* cartridge_info = (cartridge_info_t*)&req_param_data[1];
                
                            if ((cartridge_channel > 0) && (cartridge_channel < 6))
                            {
                                ret = CARTRIDGE_WriteInfo(cartridge_channel - 1, cartridge_info);
                                if (ret == false)
                                {
                                    resp_err_code = 0xE0;
                                }
                            }
                            else
                            {
                                 resp_err_code = 0xE1;
                            }
                            
                            XTCP_DMSG("<XTCP> CartridgeInfo write in channel %d: ret=%s, id= 0x%04X, Date:%04d-%02d-%02d,%02d:%02d:%02d.%03d, ExpiryDays=%d,SerialNum= %d.\r\n", 
                                                    cartridge_channel,
                                                    (ret == true) ? "successful" : "failure",
                                                    cartridge_info->id,
                                                    cartridge_info->dateOfManufacture.year + 2000,
                                                    cartridge_info->dateOfManufacture.mon,
                                                    cartridge_info->dateOfManufacture.day,
                                                    cartridge_info->dateOfManufacture.hour,
                                                    cartridge_info->dateOfManufacture.min,
                                                    cartridge_info->dateOfManufacture.sec,
                                                    cartridge_info->dateOfManufacture.msec,
                                                    cartridge_info->expiryDays,
                                                    cartridge_info->serialNumber);
                            //
                            uint8_t resp_data[2];
                            resp_data[0] = resp_err_code;
                            resp_data[1] = cartridge_channel;
                            uint16_t resp_data_len = sizeof(resp_data) / sizeof(uint8_t);
                            
                            resp_pkg_len = XTCP_MakePackage(cmd_type, cmd, resp_data_len, (uint8_t const*)&resp_data, resp_pkg);
                        }
                    }
                    break;
                    
                case CMD_CARTRIDGE_INFO_READ:
                    {
                        XTCP_DMSG("<XTCP> CMD_CARTRIDGE_INFO_READ.\r\n");
                        
                        uint16_t receive_len_should_be = sizeof(uint8_t) / sizeof(uint8_t);
                        if (param_data_len != receive_len_should_be)
                        {
                            resp_pkg_len = XTCP_MakeParamDataLenErrorRespPkg(param_data_len, receive_len_should_be, resp_pkg);
                        }
                        else
                        {
                            bool ret;
                            uint8_t cartridge_channel = req_param_data[0];

                            #pragma pack(1)
                            typedef struct
                            {
                                uint8_t err_code;
                                uint8_t channel;
                                cartridge_info_t cartridge_info;
                            } resp_cartridge_info_t;  
                            #pragma pack()
                            resp_cartridge_info_t resp_data;
                            
                            resp_data.channel = cartridge_channel;
                            
                            if ((cartridge_channel > 0)  && (cartridge_channel < 6))
                            {
                                ret = CARTRIDGE_ReadInfo(cartridge_channel - 1, &resp_data.cartridge_info);
                                if (ret == false)
                                {
                                    resp_err_code = 0xE0;
                                }
                                else
                                {
                                    ;//
                                }
                            }
                            else
                            {
                                 resp_err_code = 0xE1;
                            }
                            
                            XTCP_DMSG("<XTCP> CartridgeInfo read in channel %d: ret=%s, id= 0x%04X, %04d-%02d-%02d,%02d:%02d:%02d.%03d, ExpiryDays=%d, serialNum= %d.\r\n", 
                                                    cartridge_channel,                        
                                                    (ret == true) ? "successful" : "failure",
                                                    resp_data.cartridge_info.id,
                                                    resp_data.cartridge_info.dateOfManufacture.year + 2000,
                                                    resp_data.cartridge_info.dateOfManufacture.mon,
                                                    resp_data.cartridge_info.dateOfManufacture.day,
                                                    resp_data.cartridge_info.dateOfManufacture.hour,
                                                    resp_data.cartridge_info.dateOfManufacture.min,
                                                    resp_data.cartridge_info.dateOfManufacture.sec,
                                                    resp_data.cartridge_info.dateOfManufacture.msec,
                                                    resp_data.cartridge_info.expiryDays,
                                                    resp_data.cartridge_info.serialNumber);
                            
                            resp_data.err_code = resp_err_code;
                            uint16_t resp_data_len = sizeof(resp_data) / sizeof(uint8_t);

                            resp_pkg_len = XTCP_MakePackage(cmd_type, cmd, resp_data_len, (uint8_t const*)&resp_data, resp_pkg);
                        }
                        
                    }
                    break;
                    
                case CMD_CARTRIDGE_ENABLING_DATE_WRITE:
                    {
                        XTCP_DMSG("<XTCP> CMD_CARTRIDGE_ENABLING_DATE_WRITE.\r\n");
                        
                        uint16_t receive_len_should_be = 1 + sizeof(cartridge_enabling_date_t) / sizeof(uint8_t);
                        if (param_data_len != receive_len_should_be)
                        {
                            resp_pkg_len = XTCP_MakeParamDataLenErrorRespPkg(param_data_len, receive_len_should_be, resp_pkg);
                        }
                        else
                        {
                            bool ret = false;
                            uint8_t cartridge_channel = req_param_data[0];
                            cartridge_enabling_date_t* cartridge_data = (cartridge_enabling_date_t*)&req_param_data[1];
                
                            if ((cartridge_channel > 0) && (cartridge_channel < 6))
                            {
                                ret = CARTRIDGE_WriteEnablingDate(cartridge_channel - 1, cartridge_data);
                                if (ret == false)
                                {
                                    resp_err_code = 0xE0;
                                }
                            }
                            else
                            {
                                 resp_err_code = 0xE1;
                            }
                            
                            XTCP_DMSG("<XTCP> Cartridge EnablingDate write in channel %d: ret=%s,  Date:%04d-%02d-%02d,%02d:%02d:%02d.\r\n", 
                                                    cartridge_channel,
                                                    (ret == true) ? "successful" : "failure",

                                                    cartridge_data->enablingDate.year + 2000,
                                                    cartridge_data->enablingDate.mon,
                                                    cartridge_data->enablingDate.day,
                                                    cartridge_data->enablingDate.hour,
                                                    cartridge_data->enablingDate.min,
                                                    cartridge_data->enablingDate.sec
                                                    );
                            //
                            uint8_t resp_data[2];
                            resp_data[0] = resp_err_code;
                            resp_data[1] = cartridge_channel;
                            uint16_t resp_data_len = sizeof(resp_data) / sizeof(uint8_t);
                            
                            resp_pkg_len = XTCP_MakePackage(cmd_type, cmd, resp_data_len, (uint8_t const*)&resp_data, resp_pkg);
                        }
                    }
                    break;
                    
                case CMD_CARTRIDGE_ENABLING_DATE_READ:
                    {
                        XTCP_DMSG("<XTCP> CMD_CARTRIDGE_ENABLING_DATE_READ.\r\n");
                        
                        uint16_t receive_len_should_be = sizeof(uint8_t) / sizeof(uint8_t);
                        if (param_data_len != receive_len_should_be)
                        {
                            resp_pkg_len = XTCP_MakeParamDataLenErrorRespPkg(param_data_len, receive_len_should_be, resp_pkg);
                        }
                        else
                        {
                            bool ret;
                            uint8_t cartridge_channel = req_param_data[0];

                            #pragma pack(1)
                            typedef struct
                            {
                                uint8_t err_code;
                                uint8_t channel;
                                cartridge_enabling_date_t cartridge_data;
                                uint16_t cartridge_expiry_date;
                            } resp_cartridge_enabling_date_t;  
                            #pragma pack()
                            resp_cartridge_enabling_date_t resp_data;
                            
                            resp_data.channel = cartridge_channel;
                            
                            if ((cartridge_channel > 0)  && (cartridge_channel < 6))
                            {
                                ret = CARTRIDGE_ReadEnablingDate(cartridge_channel - 1, &resp_data.cartridge_data);
                                if (ret == false)
                                {
                                    resp_err_code = 0xE0;
                                }
                                else
                                {
                                    ;//
                                }
                                //
                                //
                                cartridge_info_t cartridge_info_data;
                                ret = CARTRIDGE_ReadInfo(cartridge_channel - 1, &cartridge_info_data);
                                resp_data.cartridge_expiry_date = cartridge_info_data.expiryDays;
                                if (ret == false)
                                {
                                    resp_err_code = 0xE0;
                                }
                                else
                                {
                                    ;//
                                }
                            }
                            else
                            {
                                 resp_err_code = 0xE0;
                            }
                            
                            XTCP_DMSG("<XTCP> Cartridge EnablingDate read in channel %d: ret=%s, %04d-%02d-%02d,%02d:%02d:%02d. expiryDate=%d.\r\n", 
                                                    cartridge_channel,                        
                                                    (ret == true) ? "successful" : "failure",
                                                    resp_data.cartridge_data.enablingDate.year + 2000,
                                                    resp_data.cartridge_data.enablingDate.mon,
                                                    resp_data.cartridge_data.enablingDate.day,
                                                    resp_data.cartridge_data.enablingDate.hour,
                                                    resp_data.cartridge_data.enablingDate.min,
                                                    resp_data.cartridge_data.enablingDate.sec,
                                                    resp_data.cartridge_expiry_date );
                            
                            resp_data.err_code = resp_err_code;
                            uint16_t resp_data_len = sizeof(resp_data) / sizeof(uint8_t);

                            resp_pkg_len = XTCP_MakePackage(cmd_type, cmd, resp_data_len, (uint8_t const*)&resp_data, resp_pkg);
                        }
                        
                    }
                    break;
                    
                default:
                    {
                        XTCP_DMSG("<XTCP> CMD_CARTRIDGE have not the cmd, -> 0x%02X.\r\n", cmd);
                        resp_pkg_len = XTCP_MakePackage(CMD_ERROR, CMD_ERROR_CMD_INVALID, 0, NULL, resp_pkg);
                    }   
                    break;
            }
            break;
            
        case CMD_TEST:
            switch (cmd)
            {
                case CMD_TEST_AIR_FAN:
                    {	
                        XTCP_DMSG("<XTCP> CMD_TEST_AIR_FAN.\r\n");
                        
                        uint16_t receive_len_should_be = 2;
                        if (param_data_len != receive_len_should_be)
                        {
                            resp_pkg_len = XTCP_MakeParamDataLenErrorRespPkg(param_data_len, receive_len_should_be, resp_pkg);
                        }
                        else
                        {
                            uint8_t channel = req_param_data[0];
                            uint8_t speed = req_param_data[1];
                           
                            if (((channel >= 0x01) && (channel <= 0x05)) || (channel == 0xFF))
                            {
                                bool ret;
                                
                                if (channel == 0xFF)
                                {
                                    for (uint8_t i = 0; i < 5; i++)
                                    {
                                        ret = FAN_SetSpeed(i, speed);
                                    }
                                }
                                else
                                {
                                    ret = FAN_SetSpeed(channel - 1, speed);
                                }
                                
                                if (ret == false)
                                {
                                    resp_err_code = 0xE0;
                                }
                            }
                            else
                            {
                                resp_err_code = 0xE0;
                            }
                            
                            XTCP_DMSG("<XTCP> FAN_TEST err_code =%d, air_fan_channel= 0x%02X, speed = 0x%02X.\r\n",
                                                        resp_err_code,
                                                        channel,
                                                        speed);
                            //
                            resp_pkg_len = XTCP_MakeErrorCodeRespPkg(cmd_type, cmd, resp_err_code, resp_pkg);
                        }                                   
                    }
                    break;
                  
                case CMD_TEST_LIGHT:
                    {
                        XTCP_DMSG("<XTCP> CMD_TEST_LIGHT.\r\n");
                        
                        uint16_t receive_len_should_be = 1 + sizeof(color_t) / sizeof(uint8_t);
                        if (param_data_len != receive_len_should_be)
                        {
                            resp_pkg_len = XTCP_MakeParamDataLenErrorRespPkg(param_data_len, receive_len_should_be, resp_pkg);
                        }
                        else
                        {
                            uint8_t channel = req_param_data[0];
                            color_t* p_color = (color_t*)&req_param_data[1];
                           
                            uint8_t pixel_num = WS2812B_GetPixelNum();  //15
                            
                            if (((channel >= 1) && (channel <= pixel_num)) || (channel == 0xFF))
                            {
                                bool ret;
                                
                                if (channel == 0xFF)
                                {
                                    WS2812B_FillColor(p_color, 0, pixel_num);
                                }
                                else
                                {
                                    WS2812B_SetPixelColor(channel - 1, p_color);
                                }
                                
                                WS2812B_Refresh();
                            }
                            else
                            {
                                resp_err_code = 0xE0;
                            }
                            
                            XTCP_DMSG("<XTCP> Test_light err_code =%d, light_channel= 0x%02X, red=0x%02X, green=0x%02X,blue=0x%02X.\r\n",
                                                        resp_err_code,
                                                        channel,
                                                        p_color->r,
                                                        p_color->g,
                                                        p_color->b);
                            //
                            resp_pkg_len = XTCP_MakeErrorCodeRespPkg(cmd_type, cmd, resp_err_code, resp_pkg);
                        }                            
                    }  
                    break;
                
                case CMD_TEST_CARTRIDGE_INFO_WRITE:
                    {
                        XTCP_DMSG("<XTCP> CMD_TEST_CARTRIDGE_INFO_WRITE.\r\n");
                        
                        uint16_t receive_len_should_be = 1 + sizeof(cartridge_info_t) / sizeof(uint8_t);
                        if (param_data_len != receive_len_should_be)
                        {
                            resp_pkg_len = XTCP_MakeParamDataLenErrorRespPkg(param_data_len, receive_len_should_be, resp_pkg);
                        }
                        else
                        {
                            bool ret;
                            uint8_t cartridge_channel = req_param_data[0];
                            cartridge_info_t* p_cartridge_info = (cartridge_info_t*)&req_param_data[1];
                            
                            if ((cartridge_channel > 0) && (cartridge_channel < 6))
                            {
                                ret = CARTRIDGE_WriteInfo(cartridge_channel - 1, p_cartridge_info);
                                if (ret == false)
                                {
                                    resp_err_code = 0xE0;
                                }
                            }
                            else
                            {
                                 resp_err_code = 0xE1;
                            }
                            
                            XTCP_DMSG("<XTCP> Test_cartridge write in channel %d: ret=%s, id= 0x%04X, Date:%04d-%02d-%02d,%02d:%02d:%02d.%03d, ExpiryDays=%d, serialNum= %d.\r\n", 
                                                    cartridge_channel,
                                                    (ret == true) ? "successful" : "failure",
                                                    p_cartridge_info->id,
                                                    p_cartridge_info->dateOfManufacture.year + 2000,
                                                    p_cartridge_info->dateOfManufacture.mon,
                                                    p_cartridge_info->dateOfManufacture.day,
                                                    p_cartridge_info->dateOfManufacture.hour,
                                                    p_cartridge_info->dateOfManufacture.min,
                                                    p_cartridge_info->dateOfManufacture.sec,
                                                    p_cartridge_info->dateOfManufacture.msec,
                                                    p_cartridge_info->expiryDays,
                                                    p_cartridge_info->serialNumber);
                            //
                              
                            //resp_pkg_len = XTCP_MakeErrorCodeRespPkg(cmd_type, cmd, resp_err_code, resp_pkg);
                            uint8_t resp_data[2];
                            resp_data[0] = resp_err_code;
                            resp_data[1] = cartridge_channel;
                            uint16_t resp_data_len = sizeof(resp_data) / sizeof(uint8_t);
                            
                            resp_pkg_len = XTCP_MakePackage(cmd_type, cmd, resp_data_len, (uint8_t const*)&resp_data, resp_pkg);
                        } 
                    }
                    break;
                    
                case CMD_TEST_CARTRIDGE_INFO_READ:
                    {
                        XTCP_DMSG("<XTCP> CMD_TEST_CARTRIDGE_INFO_READ.\r\n");
                        
                        uint16_t receive_len_should_be = 1;
                        if (param_data_len != receive_len_should_be)
                        {
                            resp_pkg_len = XTCP_MakeParamDataLenErrorRespPkg(param_data_len, receive_len_should_be, resp_pkg);
                        }
                        else
                        {
                            bool ret;
                            uint8_t cartridge_channel = req_param_data[0];

                            #pragma pack(1)
                            typedef struct
                            {
                                uint8_t err_code;
                                uint8_t cartridge_channel;
                                cartridge_info_t cartridge_info;
                            } resp_cartridge_info_t;  
                            #pragma pack()
                            
                            resp_cartridge_info_t data;
                            
                            data.cartridge_channel = cartridge_channel;
                            
                            if ((cartridge_channel > 0)  && (cartridge_channel < 6))
                            {
                                ret = CARTRIDGE_ReadInfo(cartridge_channel - 1, &data.cartridge_info);
                                if (ret == false)
                                {
                                    resp_err_code = 0xE0;
                                }
                            }
                            else
                            {
                                 resp_err_code = 0xE1;
                            }
                            
                            XTCP_DMSG("<XTCP> Test_cartridge read channel %d: ret=%s, id= 0x%04X, Date:%04d-%02d-%02d,%02d:%02d:%02d.%03d, ExpiryDays=%d, serialNum= %d.\r\n", 
                                                    cartridge_channel,                        
                                                    (ret == true) ? "successful" : "failure",
                                                    data.cartridge_info.id,
                                                    data.cartridge_info.dateOfManufacture.year + 2000,
                                                    data.cartridge_info.dateOfManufacture.mon,
                                                    data.cartridge_info.dateOfManufacture.day,
                                                    data.cartridge_info.dateOfManufacture.hour,
                                                    data.cartridge_info.dateOfManufacture.min,
                                                    data.cartridge_info.dateOfManufacture.sec,
                                                    data.cartridge_info.dateOfManufacture.msec,
                                                    data.cartridge_info.expiryDays,
                                                    data.cartridge_info.serialNumber);
                            
                            data.err_code = resp_err_code;
                            uint16_t payload_data_len = sizeof(data) / sizeof(uint8_t);

                            resp_pkg_len = XTCP_MakePackage(cmd_type, cmd, payload_data_len, (uint8_t const*)&data, resp_pkg);
                        }
                    }
                    break;
                                  
                case CMD_TEST_CARTRIDGE_ENABLING_DATE_WRITE:
                    {
                        XTCP_DMSG("<XTCP> CMD_TEST_CARTRIDGE_ENABLING_DATE_WRITE.\r\n");
                        
                        uint16_t receive_len_should_be = 1 + sizeof(cartridge_enabling_date_t) / sizeof(uint8_t);
                        if (param_data_len != receive_len_should_be)
                        {
                            resp_pkg_len = XTCP_MakeParamDataLenErrorRespPkg(param_data_len, receive_len_should_be, resp_pkg);
                        }
                        else
                        {
                            bool ret = false;
                            uint8_t cartridge_channel = req_param_data[0];
                            cartridge_enabling_date_t* cartridge_data = (cartridge_enabling_date_t*)&req_param_data[1];
                
                            if ((cartridge_channel > 0) && (cartridge_channel < 6))
                            {
                                ret = CARTRIDGE_WriteEnablingDate(cartridge_channel - 1, cartridge_data);
                                if (ret == false)
                                {
                                    resp_err_code = 0xE0;
                                }
                            }
                            else
                            {
                                 resp_err_code = 0xE1;
                            }
                            
                            XTCP_DMSG("<XTCP> Test Cartridge EnablingDate write in channel %d: ret=%s,  Date:%04d-%02d-%02d,%02d:%02d:%02d.\r\n", 
                                                    cartridge_channel,
                                                    (ret == true) ? "successful" : "failure",

                                                    cartridge_data->enablingDate.year + 2000,
                                                    cartridge_data->enablingDate.mon,
                                                    cartridge_data->enablingDate.day,
                                                    cartridge_data->enablingDate.hour,
                                                    cartridge_data->enablingDate.min,
                                                    cartridge_data->enablingDate.sec
                                                    );
                            //
                            uint8_t resp_data[2];
                            resp_data[0] = resp_err_code;
                            resp_data[1] = cartridge_channel;
                            uint16_t resp_data_len = sizeof(resp_data) / sizeof(uint8_t);
                            
                            resp_pkg_len = XTCP_MakePackage(cmd_type, cmd, resp_data_len, (uint8_t const*)&resp_data, resp_pkg);
                        }
                    }
                    break;
                    
                case CMD_TEST_CARTRIDGE_ENABLING_DATE_READ:
                    {
                        XTCP_DMSG("<XTCP> CMD_TEST_CARTRIDGE_ENABLING_DATE_READ.\r\n");
                        
                        uint16_t receive_len_should_be = sizeof(uint8_t) / sizeof(uint8_t);
                        if (param_data_len != receive_len_should_be)
                        {
                            resp_pkg_len = XTCP_MakeParamDataLenErrorRespPkg(param_data_len, receive_len_should_be, resp_pkg);
                        }
                        else
                        {
                            bool ret;
                            uint8_t cartridge_channel = req_param_data[0];

                            #pragma pack(1)
                            typedef struct
                            {
                                uint8_t err_code;
                                uint8_t channel;
                                cartridge_enabling_date_t cartridge_data;
                                uint16_t cartridge_expiry_date;
                            } resp_cartridge_enabling_date_t;  
                            #pragma pack()
                            resp_cartridge_enabling_date_t resp_data;
                            
                            resp_data.channel = cartridge_channel;
                            
                            if ((cartridge_channel > 0)  && (cartridge_channel < 6))
                            {
                                ret = CARTRIDGE_ReadEnablingDate(cartridge_channel - 1, &resp_data.cartridge_data);
                                if (ret == false)
                                {
                                    resp_err_code = 0xE0;
                                }
                                else
                                {
                                    ;//
                                }
                                //
                                cartridge_info_t cartridge_info_data;
                                ret = CARTRIDGE_ReadInfo(cartridge_channel - 1, &cartridge_info_data);
                                resp_data.cartridge_expiry_date = cartridge_info_data.expiryDays;
                                if (ret == false)
                                {
                                    resp_err_code = 0xE0;
                                }
                                else
                                {
                                    ;//
                                }
                            }
                            else
                            {
                                 resp_err_code = 0xE1;
                            }
                            
                            XTCP_DMSG("<XTCP> Test Cartridge EnablingDate read in channel %d: ret=%s, %04d-%02d-%02d,%02d:%02d:%02d. expirtyDate=%d.\r\n", 
                                                    cartridge_channel,                        
                                                    (ret == true) ? "successful" : "failure",
                                                    resp_data.cartridge_data.enablingDate.year + 2000,
                                                    resp_data.cartridge_data.enablingDate.mon,
                                                    resp_data.cartridge_data.enablingDate.day,
                                                    resp_data.cartridge_data.enablingDate.hour,
                                                    resp_data.cartridge_data.enablingDate.min,
                                                    resp_data.cartridge_data.enablingDate.sec,
                                                    resp_data.cartridge_expiry_date);
                            
                            resp_data.err_code = resp_err_code;
                            uint16_t resp_data_len = sizeof(resp_data) / sizeof(uint8_t);

                            resp_pkg_len = XTCP_MakePackage(cmd_type, cmd, resp_data_len, (uint8_t const*)&resp_data, resp_pkg);
                        }
                        
                    }
                    break;

                default:
                    {
                        XTCP_DMSG("<XTCP> CMD_TEST have not the cmd, -> 0x%02X.\r\n", cmd);
                        resp_pkg_len = XTCP_MakePackage(CMD_ERROR, CMD_ERROR_CMD_INVALID, 0, NULL, resp_pkg);
                    }
                    break;
             }
             break;

        default:
            {
                XTCP_DMSG("<XTCP> have not the cmd_type, -> 0x%02X.\r\n", cmd_type);
                resp_pkg_len = XTCP_MakePackage(CMD_ERROR, CMD_ERROR_CMD_TYPE_INVALID, 0, NULL, resp_pkg);
            }
            break;         
    }

    return resp_pkg_len;
}



typedef uint32_t (*p_GetRxFifoLenFunc_t)(void);
typedef app_fifo_t* (*p_GetRxFifoPtrFunc_t)(void);

#define RX_PACKET_SIZE	256
#define TX_PACKET_SIZE	256
typedef struct potocol_parse_param_t {
    p_GetRxFifoLenFunc_t GetFifoLen;
    p_GetRxFifoPtrFunc_t GetRxFifoPtr;
    uint8_t rx_packet_buf[RX_PACKET_SIZE + 1];
    uint16_t rx_packet_len;
    uint8_t is_packet_header;
    uint8_t is_packet_phrase;
    uint32_t payload_data_len;
    
    uint8_t tx_packet_buf[TX_PACKET_SIZE+1];
    uint16_t txPacketLen;
} potocol_parse_param_t;


potocol_parse_param_t paket_p[2] = {0};

// 7E cmd_type cmd dataLenLo8 dataLenHi8 cmd_data data_xor 7F

uint16_t XTCP_PotocalParse(potocol_parse_param_t* packet)
{
    uint16_t data_len = 0;
    uint32_t err_code;	
	uint8_t rx_buf[2] = {0, 0};

    uint16_t try_cnt = packet->GetFifoLen();
    
    while (try_cnt)
	{
        app_fifo_t *rx_fifo = packet->GetRxFifoPtr();
		err_code = app_fifo_get(rx_fifo, &rx_buf[0]);
		if (err_code == NRF_SUCCESS)
		{
			//XTCP_DMSG("Y");
            XTCP_DMSG("%02X,", rx_buf[0]);
			try_cnt--;

			// If the header parse is ok.
			if (packet->is_packet_header)
			{
                if (packet->rx_packet_len >= RX_PACKET_SIZE)
                {
                     packet->is_packet_header = 0;
                }
                else
                {
                    packet->rx_packet_buf[packet->rx_packet_len++] = rx_buf[0]; //0 1 2
                    
                    // have one package. 
                    if (rx_buf[0] == 0x7F)
                    {
                        packet->is_packet_header = 0; 
                        try_cnt = 0;   // 
                        packet->txPacketLen = XTCP_ParsePackage(packet->rx_packet_buf, packet->rx_packet_len, packet->tx_packet_buf, TX_PACKET_SIZE + 1);
                        data_len = packet->txPacketLen;
                    }
                }
			}
  
            // have the package header.
            if (rx_buf[0] == 0x7E)
			{
				packet->is_packet_header = 1;
				packet->rx_packet_len = 0;
				packet->rx_packet_buf[packet->rx_packet_len++] = 0x7E;
				XTCP_DMSG("=.=");
			}
            
            packet->is_packet_phrase = 0;
		}
		else
		{
			try_cnt = 0;
			//
		}
	}
    return data_len;
}


void XTCP_Loop(void)
{
    uint32_t err_code = 0;
    volatile uint16_t packet_resp_len = 0;
    uint8_t packet_sent_is_finish = 0; 
    
    paket_p[0].GetFifoLen = UART_GetRxFifoLen;
    paket_p[0].GetRxFifoPtr = UART_GetRxFifoPtr;
    
    paket_p[1].GetFifoLen = BLE_GetRxFifoLen;
    paket_p[1].GetRxFifoPtr = BLE_GetRxFifoPtr;
   
    // UART Potocal parase.
    packet_resp_len = 0;
    packet_resp_len = XTCP_PotocalParse(&paket_p[0]);
    if (packet_resp_len)
    {
        if (f.BLE_CONNECT_STATUS == BLE_STATUS_FAST_ADV)
        {
            LIGHT_SequenceSet(LIGHT_DO_NOTHING);
            err_code = BLE_AdvStop();
        }
        // UART send.         
        err_code = UART_PacketSend((uint8_t *)&paket_p[0].tx_packet_buf, paket_p[0].txPacketLen, &packet_sent_is_finish); 
    }
    
    // BLE Potocal parase.
    packet_resp_len = 0;
    packet_resp_len = XTCP_PotocalParse(&paket_p[1]);
    if (packet_resp_len)
    {
        // BLE_send.     
        err_code = BLE_PacketSend((uint8_t *)&paket_p[1].tx_packet_buf, paket_p[1].txPacketLen, &packet_sent_is_finish);      
    }
}


/******************* (C) COPYRIGHT 2019 All Rights Reserved *****END OF FILE****/
