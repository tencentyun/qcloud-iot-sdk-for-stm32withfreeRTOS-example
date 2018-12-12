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
#include <limits.h>
#include <stdbool.h>
#include <string.h>

#include "stm32f1xx_hal.h"
#include "qcloud_iot_export.h"
#include "qcloud_iot_import.h"
#include "lite-utils.h"
//#include "stm32f10x_flash.h"
#include "peripheral_drv.h"



#define YEILD_TIMEOUT_MS		(3000)
#define RETRY_TIMES				(3)

/* 产品名称, 与云端同步设备状态时需要  */
#define QCLOUD_IOT_MY_PRODUCT_ID            "117ZVPFPXY"
/* 设备名称, 与云端同步设备状态时需要 */
#define QCLOUD_IOT_MY_DEVICE_NAME           "ED186B57D483"

#ifdef AUTH_MODE_CERT
    /* 客户端证书文件名  非对称加密使用*/
    #define QCLOUD_IOT_CERT_FILENAME          "YOUR_DEVICE_NAME_cert.crt"
    /* 客户端私钥文件名 非对称加密使用*/
    #define QCLOUD_IOT_KEY_FILENAME           "YOUR_DEVICE_NAME_private.key"

    static char sg_cert_file[PATH_MAX + 1];      //客户端证书全路径
    static char sg_key_file[PATH_MAX + 1];       //客户端密钥全路径

#else
    #define QCLOUD_IOT_DEVICE_SECRET                  "ZPMbcgJlbypTEVsv4yQB3g=="
#endif




#define MAX_NICK_LEN					 	(64)
#define MAX_LENGTH_OF_UPDATE_JSON_BUFFER 	(200)
#define MAX_RECV_LEN 					 	(512 + 1)

#define PAYLOAD_STATE_DESIRE_NICK			"payload.state.desired.Nick"
#define PAYLOAD_STATE_REPORTED_NICK			"payload.state.reported.Nick"

static char sg_report_nickname[MAX_NICK_LEN]={0};
static char sg_desire_nickname[MAX_NICK_LEN];
uint8_t sg_display_localname[MAX_NICK_LEN];


static DeviceProperty sg_local_name_prop;
static DeviceProperty sg_nick_desire_prop;

static bool sg_message_arrived_on_delta = false;

static MQTTEventType sg_subscribe_event_result = MQTT_EVENT_UNDEF;
static bool sg_finish_shadow_get = false;
static bool sg_finish_shadow_update = true;

char sg_document_buffer[MAX_LENGTH_OF_UPDATE_JSON_BUFFER];
size_t sg_document_buffersize = sizeof(sg_document_buffer) / sizeof(sg_document_buffer[0]);
smanuInfo g_DevInfo;


static int _register_config_shadow_property(void *client);
extern void mem_info(void);


/**
 * 是否更新显示信息
 */
static bool displayNameNeedUpdate(char *localName) 
{
	int rc = false;
		
	if(0 == strlen(sg_desire_nickname))
	{
		if((0 != strcmp(sg_report_nickname, localName))&&(strlen(sg_report_nickname) > 0))
		{
			memset(localName, 0, MAX_NICK_LEN);
			strncpy(localName, sg_report_nickname, MAX_NICK_LEN);
			rc = true;
		}		
	}
	else
	{
		memset(localName, 0, MAX_NICK_LEN);
		strncpy(localName, sg_desire_nickname, MAX_NICK_LEN);
		memset(sg_desire_nickname, 0, MAX_NICK_LEN);
		memset(sg_report_nickname, 0, MAX_NICK_LEN);
		rc = true;
	}

	return rc;
}

/**
 * 文档操作请求返回回调函数
 */
static void on_request_handler(void *pClient, Method method, RequestAck status, char *jsonDoc, void *userData) 
{

    char *shadow_status = NULL;
    char *shadow_method = NULL;
	char *pdesireName;
	char *preportName;

    if (status == ACK_TIMEOUT) {
        shadow_status = "ACK_TIMEOUT";
    } else if (status == ACK_ACCEPTED) {
        shadow_status = "ACK_ACCEPTED";
    } else if (status == ACK_REJECTED) {
        shadow_status = "ACK_REJECTED";
    }

    if (method == GET) {
        shadow_method = "GET";
    } else if (method == UPDATE) {
        shadow_method = "UPDATE";
    }

    Log_i("Method=%s|Ack=%s", shadow_method, shadow_status);
    Log_i("received jsonString=%s", jsonDoc);

    if (status == ACK_ACCEPTED && method == UPDATE) {
        if (sg_message_arrived_on_delta)  // 等待清空desired字段完成
            sg_message_arrived_on_delta = false;
		
		sg_finish_shadow_update = true;
        Log_i("Update Shadow Success");
		
    } else if (status == ACK_ACCEPTED && method == GET) {
        Log_i("Get Shadow Document Success");	
		
		pdesireName = LITE_json_value_of(PAYLOAD_STATE_DESIRE_NICK,jsonDoc);	

		if(NULL != pdesireName)
		{
			Log_i("desireName=%s", pdesireName);
			memset(sg_desire_nickname, 0, MAX_NICK_LEN);
			strncpy(sg_desire_nickname, pdesireName, MAX_NICK_LEN);	 
			sg_message_arrived_on_delta = true;
		}
		else
		{
			preportName = LITE_json_value_of(PAYLOAD_STATE_REPORTED_NICK,jsonDoc);		
			if(NULL != preportName)
			{
				Log_i("reportName=%s", preportName);
				memset(sg_report_nickname, 0, MAX_NICK_LEN);
				strncpy(sg_report_nickname, preportName, MAX_NICK_LEN);	       
			}
		}

		
		sg_finish_shadow_get = true;

    //} else if (status == ACK_TIMEOUT && (method == UPDATE)) {
    } else if (status == ACK_TIMEOUT ) {
        Log_e("ACK_TIMEOUT!");
		sg_finish_shadow_get = true;
        HAL_NVIC_SystemReset();
    }
}


/**
 * delta消息回调处理函数
 */
static void on_nick_name_callback(void *pClient, char *jsonResponse, uint32_t responseLen, DeviceProperty *context) 
{
	char *pName;
	
    Log_i("nick callback jsonString=%s|dataLen=%u", jsonResponse, responseLen);

    if (context != NULL) {
		pName = LITE_json_value_of(context->key,jsonResponse);
        Log_i("modify desire nickname to: %s", pName);
        sg_message_arrived_on_delta = true;
		strncpy(sg_desire_nickname, pName, MAX_NICK_LEN);
    }
}


static void event_handler(void *pclient, void *handle_context, MQTTEventMsg *msg) 
{	
	uintptr_t packet_id = (uintptr_t)msg->msg;

	switch(msg->event_type) {
		case MQTT_EVENT_UNDEF:
			Log_i("undefined event occur.");
			break;

		case MQTT_EVENT_DISCONNECT:
			Log_i("MQTT disconnect.");
			break;

		case MQTT_EVENT_RECONNECT:
			Log_i("MQTT reconnect.");
			break;

		case MQTT_EVENT_SUBCRIBE_SUCCESS:
            sg_subscribe_event_result = msg->event_type;
			Log_i("subscribe success, packet-id=%u", (unsigned int)packet_id);
			break;

		case MQTT_EVENT_SUBCRIBE_TIMEOUT:
            sg_subscribe_event_result = msg->event_type;
			Log_i("subscribe wait ack timeout, packet-id=%u", (unsigned int)packet_id);
			break;

		case MQTT_EVENT_SUBCRIBE_NACK:
            sg_subscribe_event_result = msg->event_type;
			Log_i("subscribe nack, packet-id=%u", (unsigned int)packet_id);
			break;

		case MQTT_EVENT_PUBLISH_SUCCESS:
			Log_i("publish success, packet-id=%u", (unsigned int)packet_id);
			break;

		case MQTT_EVENT_PUBLISH_TIMEOUT:
			Log_i("publish timeout, packet-id=%u", (unsigned int)packet_id);
			break;

		case MQTT_EVENT_PUBLISH_NACK:
			Log_i("publish nack, packet-id=%u", (unsigned int)packet_id);
			break;
		default:
			Log_i("Should NOT arrive here.");
			break;
	}
}

/**
 * report energy consumption
 */
static int _do_report_localname(void *client) 
{
	int rc ;	
    rc = IOT_Shadow_JSON_ConstructReport(client, sg_document_buffer, sg_document_buffersize, 1, &sg_local_name_prop);

    if (rc != QCLOUD_ERR_SUCCESS) {
    	Log_e("shadow construct report failed: %d", rc);
        return rc;
    }

    Log_i("Update Shadow: %s", sg_document_buffer);
    rc = IOT_Shadow_Update(client, sg_document_buffer, sg_document_buffersize, on_request_handler, NULL, QCLOUD_IOT_MQTT_COMMAND_TIMEOUT);

    if (rc != QCLOUD_ERR_SUCCESS) {
        Log_i("Update Shadow Failed: %d", rc);
        return rc;
    }

    return rc;
}

static int _do_report_nick_desire(void *client) 
{
    /*
     * 如果收到delta消息，或者get时有desired内容，那么需要将 所有 的属性执行完毕之后，上报清空desried消息，否则后台的记录一直存在。
     * 如果没有执行变更属性值操作，那么不用上报desired字段。
     */
    int rc = IOT_Shadow_JSON_ConstructReportAndDesireAllNull(client, sg_document_buffer, sg_document_buffersize, 1, &sg_nick_desire_prop);
	
	if (rc != QCLOUD_ERR_SUCCESS) {
		Log_e("shadow construct report failed: %d", rc);
		return rc;
	}

	Log_i("update desire nick: %s", sg_document_buffer);
	rc = IOT_Shadow_Update(client, sg_document_buffer, sg_document_buffersize, on_request_handler, NULL, QCLOUD_IOT_MQTT_COMMAND_TIMEOUT);

    if (rc != QCLOUD_ERR_SUCCESS) {
        Log_i("Update Shadow Failed: %d", rc);
        return rc;
    } else {
        Log_i("Update Shadow Success");
    }

    return rc;
}

/**
 * 设置MQTT connet初始化参数
 */
static int _setup_connect_init_params(ShadowInitParams* initParams)
{
//	initParams->device_name = QCLOUD_IOT_MY_DEVICE_NAME;
//	initParams->product_id = QCLOUD_IOT_MY_PRODUCT_ID;
	initParams->device_name = g_DevInfo.devName;
	initParams->product_id = g_DevInfo.productId;


#ifdef AUTH_MODE_CERT
    // 获取CA证书、客户端证书以及私钥文件的路径
    char certs_dir[PATH_MAX + 1] = "certs";
    char current_path[PATH_MAX + 1];
    char *cwd = getcwd(current_path, sizeof(current_path));
    if (cwd == NULL)
    {
        Log_e("getcwd return NULL");
        return QCLOUD_ERR_FAILURE;
    }
    sprintf(sg_cert_file, "%s/%s/%s", current_path, certs_dir, QCLOUD_IOT_CERT_FILENAME);
    sprintf(sg_key_file, "%s/%s/%s", current_path, certs_dir, QCLOUD_IOT_KEY_FILENAME);

    initParams->cert_file = sg_cert_file;
    initParams->key_file = sg_key_file;
#else
//    initParams->device_secret = QCLOUD_IOT_DEVICE_SECRET;
	 initParams->device_secret = g_DevInfo.devSerc;
#endif

	initParams->auto_connect_enable = 1;
    initParams->event_handle.h_fp = event_handler;

    return QCLOUD_ERR_SUCCESS;
}

/**
 * 设置shadow相关属性参数
 */
static void _setup_shadow_data()
{
    //s_localNameReportProp: device ---> shadow <---> app
    sg_local_name_prop.key = "localName";
    sg_local_name_prop.data = &sg_display_localname;
    sg_local_name_prop.type = JSTRING;
    //s_nickDesireProp: app ---> shadow ---> device
    sg_nick_desire_prop.key = "Nick";
    sg_nick_desire_prop.data = &sg_desire_nickname;
    sg_nick_desire_prop.type = JSTRING;
}

/**
 * 注册shadow配置型属性
 */
static int _register_config_shadow_property(void *client)
{
    return IOT_Shadow_Register_Property(client, &sg_nick_desire_prop, on_nick_name_callback);
}

void exhibitor_shadow_task(void const * argument) 
{
    int rc;


    //init log level
    IOT_Log_Set_Level(LOG_DEBUG);

	Log_i("exhibitor_shadow_task started");
	Log_i("\n\rRegister net work...");
	if(QCLOUD_ERR_SUCCESS != NetInit())
	{
		goto end;
	}

	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_SET);  // singal for net ok
	mem_info();
    //init connection
    ShadowInitParams init_params = DEFAULT_SHAWDOW_INIT_PARAMS;
    rc = _setup_connect_init_params(&init_params);
    if (rc != QCLOUD_ERR_SUCCESS) 
	{
		goto end;
	}

    void *client = IOT_Shadow_Construct(&init_params);
    if (client != NULL) {
        Log_i("Cloud Device Construct Success");
    } 
	else 
	{
        Log_e("Cloud Device Construct Failed");
        goto end;
    }

    //init shadow data
    _setup_shadow_data();

    //register config shadow propertys here
    rc = _register_config_shadow_property(client);
    if (rc == QCLOUD_ERR_SUCCESS) {
        Log_i("Cloud Device Register Delta Success");
    } 
	else 
	{
        Log_e("Cloud Device Register Delta Failed: %d", rc);
        goto end;
    }

	/*Sync data during offline*/
    rc = IOT_Shadow_Get(client, on_request_handler, NULL, QCLOUD_IOT_MQTT_COMMAND_TIMEOUT);
	if(QCLOUD_ERR_SUCCESS != rc)
	{
		Log_e("Shadow Get error, errCode:%d", rc);
		goto end;
	}

    while(sg_finish_shadow_get == false)
    {
        Log_i("Wait for Shadow Get Result");
		rc = IOT_Shadow_Yield(client, 2*YEILD_TIMEOUT_MS);
        if(QCLOUD_ERR_SUCCESS != rc)
        {
        	Log_e("Shadow sync data err,errCode: %d", rc);
			goto end;
		}
        osDelay(1000);
    }

	/*You can also subcribe your mqtt topic here*/

	mem_info();
	
    while (IOT_Shadow_IsConnected(client) || rc == QCLOUD_ERR_MQTT_ATTEMPTING_RECONNECT 
			|| rc == QCLOUD_ERR_MQTT_RECONNECTED) 
	{

		rc = IOT_Shadow_Yield(client, YEILD_TIMEOUT_MS);
        if (rc == QCLOUD_ERR_MQTT_ATTEMPTING_RECONNECT) 
		{
            osDelay(1000);
            continue;
        }

		if (sg_message_arrived_on_delta == true)
        {
			rc = _do_report_nick_desire(client);
			//memset(sg_desire_nickname, 0, MAX_NICK_LEN);
		}


		else if (rc != QCLOUD_ERR_SUCCESS) 
		{
			Log_e("Exit1 loop caused of errCode: %d", rc);
		}

	
		if(true == displayNameNeedUpdate(sg_display_localname))
		{	
			showName(sg_display_localname);
			if(QCLOUD_ERR_SUCCESS == saveNameToFlash(sg_display_localname))
			{
				Log_i("Save name to flash success");
			}
			else
			{
				Log_i("Save name to flash fail");
			}	
			Log_i("displayName refreshed");
		}
        Log_i("displayName: %s, reportName:%s, desireName:%s", sg_display_localname, sg_report_nickname, sg_desire_nickname);
		

		if(rc != QCLOUD_ERR_SUCCESS)
		{
			Log_e("Exit2 loop caused of errCode: %d", rc);
			goto end;
		}

   		//You can do some work for power save  here if needed
        osDelay(3000);
    }
	
end:
    //rc = IOT_Shadow_Destroy(client);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_RESET);  // singal for net nok
    HAL_NVIC_SystemReset();

	
    return;
}
