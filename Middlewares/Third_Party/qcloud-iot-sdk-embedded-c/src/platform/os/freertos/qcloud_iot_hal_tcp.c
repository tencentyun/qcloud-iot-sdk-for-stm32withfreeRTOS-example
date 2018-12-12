/*
 * Tencent is pleased to support the open source community by making IoT Hub available.
 * Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

 * Licensed under the MIT License (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://opensource.org/licenses/MIT

 * Unless required by applicable law or agreed to in writing, software distributed under the License is
 * distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <stdio.h>
#include <string.h>
#include "stm32f1xx_hal.h"
#include "qcloud_iot_import.h"
#include "qcloud_iot_export_log.h"
#include "qcloud_iot_export_error.h"

#define IPPROTO_TCP     6
#define IPPROTO_UDP     17



intptr_t HAL_TCP_Connect(const char *host, uint16_t port)
{
	int fd;

	Log_i("HAL_TCP_Connect entry, host=%s port=%d\r\n", host , port);
     
	if(HAL_OK == net_connect_by_at(host, port, &fd, IPPROTO_TCP, NULL)) 
	{
		Log_i("net connect success\n\r");
	}
	else
	{	
		Log_i("net connect fail\n\r");	
		if(HAL_OK == net_init_by_at())  /*重新初始化模组*/
		{
			Log_i("net reinit success\n\r");	
			if(HAL_OK == net_connect_by_at(host, port, &fd, IPPROTO_TCP, NULL)) 
			{
				Log_i("net connect success\n\r");
			}
			else
			{
				Log_i("net connect fail\n\r");
				fd = -1; 
			}
		}	
		else
		{
			Log_i("net reinit fail\n\r");	
			fd = -1; 
		}	
	}

	
	
	//return (fd > 0)?fd:0;
	return fd;
}


int HAL_TCP_Disconnect(uintptr_t fd)
{
	return net_disconnect_by_at(fd, NULL);
}


int HAL_TCP_Write(uintptr_t fd, const unsigned char *buf, uint32_t len, uint32_t timeout_ms, size_t *written_len)
{
	int Ret;
//	Log_i("HAL_TCP_Write len  %d timeout %d\r\n", len , timeout_ms);
	
	*written_len = net_write_by_at(fd, buf, len, timeout_ms);

	return  (*(int *)written_len > 0)?QCLOUD_ERR_SUCCESS:(*(int *)written_len);
}


int HAL_TCP_Read(uintptr_t fd, unsigned char *buf, uint32_t len, uint32_t timeout_ms, size_t *read_len)
{
	int ret;
	
//	Log_i("HAL_TCP_Read len %d timeout %d\r\n", len , timeout_ms);													
	ret = net_read_by_at(fd, buf, len, timeout_ms);

	if(0 == ret)
	{
		ret = QCLOUD_ERR_SSL_NOTHING_TO_READ;
	}
	
	return (ret > 0)?QCLOUD_ERR_SUCCESS:ret;

}
