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
#include <stdlib.h>
#include <string.h>
#include "qcloud_iot_import.h"
#include "qcloud_iot_export_log.h"
#include "qcloud_iot_export_error.h"

uintptr_t HAL_UDP_Connect(const char *host, unsigned short port)
{

	//To DO
	
	return 0;
}

void HAL_UDP_Disconnect(uintptr_t fd)
{
	//To DO	
}

int HAL_UDP_Write(uintptr_t fd, const unsigned char *p_data, unsigned int datalen)
{
	//To DO
	
	return 0;

}

int HAL_UDP_Read(uintptr_t fd, unsigned char *p_data, unsigned int datalen)
{
	//To DO
	
	return 0;

}

int HAL_UDP_ReadTimeout(uintptr_t fd, unsigned char *p_data, unsigned int datalen, unsigned int timeout_ms)
{
 	//To DO
	
	return 0;
}
