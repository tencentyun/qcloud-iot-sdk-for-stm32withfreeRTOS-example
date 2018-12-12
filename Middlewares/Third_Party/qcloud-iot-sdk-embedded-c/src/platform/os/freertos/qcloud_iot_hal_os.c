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
#include <stdarg.h>
#include "cmsis_os.h"
#include "qcloud_iot_import.h"


/* Knuth's TAOCP section 3.6 */
#define	M	((1U<<31) -1)
#define	A	48271
#define	Q	44488		// M/A
#define	R	3399		// M%A; R < Q !!!

/*Global value*/
static unsigned int _seed=1;

int rand_r(unsigned int* seed)
{   int32_t X;

    X = *seed;
    X = A*(X%Q) - R * (int32_t) (X/Q);
    if (X < 0)
	X += M;

    *seed = X;
    return X;
}

int rand(void) {
  return rand_r(&_seed);
}

void srand(unsigned int i)
{ 
	_seed=i;
}

void HAL_Printf(_IN_ const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);

//    fflush(stdout);
}

//int HAL_Snprintf(_IN_ char *str, const int len, const char *fmt, ...)
//{
//    va_list args;
//    int rc;

//    va_start(args, fmt);
//    rc = vsnprintf(str, len, fmt, args);
//    va_end(args);

//    return rc;
//}

void *HAL_Malloc(_IN_ uint32_t size)
{
	return pvPortMalloc( size);
}

void HAL_Free(_IN_ void *ptr)
{
    vPortFree(ptr);
}

int HAL_Vsnprintf(_IN_ char *str, _IN_ const int len, _IN_ const char *format, va_list ap)
{
    return vsnprintf(str, len, format, ap);
}

void HAL_SleepMs(_IN_ uint32_t ms)
{
   (void)osDelay(ms);
}

void HAL_DelayMs(_IN_ uint32_t ms)
{
   (void)HAL_Delay(ms);
}


osMutexId g_Mutex;

void *HAL_MutexCreate(void)
 {
	int test;
	
	return (void *)osMutexCreate (NULL);
}

void HAL_MutexDestroy(_IN_ osMutexId mutex)
{
	osStatus ret;
	
    if(osOK != (ret = osMutexDelete(mutex)))
    {
		HAL_Printf("HAL_MutexDestroy err, err:%d\n\r",ret);
	}
}

void HAL_MutexLock(_IN_ osMutexId mutex)
{
	osStatus ret;

	if(osOK != (ret = osMutexWait(mutex, osWaitForever)))
	{
		HAL_Printf("HAL_MutexLock err, err:%d\n\r",ret);
	}
}

void HAL_MutexUnlock(_IN_ osMutexId mutex)
{
	osStatus ret;

	if(osOK != (ret = osMutexRelease(mutex)))
	{
		HAL_Printf("HAL_MutexUnlock err, err:%d\n\r",ret);
	}	
}

