/*
 *  Copyright (C) 2012, Samsung Electronics Co. Ltd. All Rights Reserved.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 */
#include "ssp.h"

/* SSP -> AP Instruction */
#define MSG2AP_INST_BYPASS_DATA			0x37
#define MSG2AP_INST_LIBRARY_DATA		0x01
#define MSG2AP_INST_DEBUG_DATA			0x03
#define MSG2AP_INST_BIG_DATA			0x04
#define MSG2AP_INST_META_DATA			0x05
#define MSG2AP_INST_TIME_SYNC			0x06
#define MSG2AP_INST_RESET			0x07

/*************************************************************************/
/* SSP parsing the dataframe                                             */
/*************************************************************************/

static void get_timestamp(struct ssp_data *data, char *pchRcvDataFrame,
		int *iDataIdx, struct sensor_value *sensorsdata)
{
	s32 otimestamp = 0;
	s64 ctimestamp = 0;

	memcpy(&otimestamp, pchRcvDataFrame + *iDataIdx, 4);
	*iDataIdx += 4;

	ctimestamp = (s64) otimestamp * 1000000;
	sensorsdata->timestamp = data->timestamp + ctimestamp;
}

static void get_3axis_sensordata(char *pchRcvDataFrame, int *iDataIdx,
	struct sensor_value *sensorsdata)
{
	memcpy(sensorsdata, pchRcvDataFrame + *iDataIdx, 6);
	*iDataIdx += 6;
}

#ifdef CONFIG_SENSORS_SSP_ADPD142
static void get_hrm_raw_sensordata(char *pchRcvDataFrame, int *iDataIdx,
	struct sensor_value *sensorsdata)
{
	memcpy(sensorsdata, pchRcvDataFrame + *iDataIdx, 8);
	*iDataIdx += 8;
}

static void get_hrm_raw_fac_sensordata(char *pchRcvDataFrame, int *iDataIdx,
	struct sensor_value *sensorsdata)
{
	memcpy(sensorsdata, pchRcvDataFrame + *iDataIdx, 36);
	*iDataIdx += 36;
}

static void get_hrm_lib_sensordata(char *pchRcvDataFrame, int *iDataIdx,
	struct sensor_value *sensorsdata)
{
	memcpy(sensorsdata, pchRcvDataFrame + *iDataIdx, 8);
	*iDataIdx += 8;
}
#endif

int handle_big_data(struct ssp_data *data, char *pchRcvDataFrame, int *pDataIdx) {
	u8 bigType = 0;
	struct ssp_big *big = kzalloc(sizeof(*big), GFP_KERNEL);
	big->data = data;
	bigType = pchRcvDataFrame[(*pDataIdx)++];
	memcpy(&big->length, pchRcvDataFrame + *pDataIdx, 4);
	*pDataIdx += 4;
	memcpy(&big->addr, pchRcvDataFrame + *pDataIdx, 4);
	*pDataIdx += 4;

	if (bigType >= BIG_TYPE_MAX) {
		kfree(big);
		return FAIL;
	}

	INIT_WORK(&big->work, data->ssp_big_task[bigType]);
	queue_work(data->debug_wq, &big->work);
	return SUCCESS;
}

void refresh_task(struct work_struct *work)
{
	struct ssp_data *data = container_of((struct delayed_work *)work,
			struct ssp_data, work_refresh);

	func_dbg();
	data->uResetCnt++;
	if (initialize_mcu(data) > 0) {
		sync_sensor_state(data);
		ssp_sensorhub_report_notice(data, MSG2SSP_AP_STATUS_RESET);
		if (data->uLastAPState != 0)
			ssp_send_cmd(data, data->uLastAPState, 0);
		if (data->uLastResumeState != 0)
			ssp_send_cmd(data, data->uLastResumeState, 0);
		data->uTimeOutCnt = 0;
	}
}

int queue_refresh_task(struct ssp_data *data, int delay)
{
	cancel_delayed_work_sync(&data->work_refresh);

	INIT_DELAYED_WORK(&data->work_refresh, refresh_task);
	queue_delayed_work(data->debug_wq, &data->work_refresh,
			msecs_to_jiffies(delay));
	return SUCCESS;
}

int parse_dataframe(struct ssp_data *data, char *pchRcvDataFrame, int iLength)
{
	int iDataIdx, iSensorData;
	u16 length = 0;
	struct sensor_value sensorsdata;
	struct timespec ts;
	int iRet = FAIL;

	getnstimeofday(&ts);

	for (iDataIdx = 0; iDataIdx < iLength;) {
		switch (pchRcvDataFrame[iDataIdx++]) {
		case MSG2AP_INST_BYPASS_DATA:
			iSensorData =
				pchRcvDataFrame[iDataIdx++];
			if ((iSensorData < 0) || (iSensorData >= SENSOR_MAX)) {
				ssp_err("MCU data frame1 error %d\n", iSensorData);
				return ERROR;
			}
			data->get_sensor_data[iSensorData](pchRcvDataFrame,
					&iDataIdx, &sensorsdata);
			get_timestamp(data, pchRcvDataFrame, &iDataIdx,
					&sensorsdata);
			data->report_sensor_data[iSensorData](data,
					&sensorsdata);
			break;
		case MSG2AP_INST_DEBUG_DATA:
			iSensorData =
				print_mcu_debug(pchRcvDataFrame, &iDataIdx, iLength);
			if (iSensorData) {
				ssp_err("MCU data frame3 error %d\n", iSensorData);
				return ERROR;
			}
			break;
		case MSG2AP_INST_LIBRARY_DATA:
			memcpy(&length, pchRcvDataFrame + iDataIdx, 2);
			iDataIdx += 2;
			if (data->bLpModeEnabled == true)
				iRet = ssp_parse_motion(data, pchRcvDataFrame,
							iDataIdx, iDataIdx + length);
			if (iRet == FAIL)
				ssp_sensorhub_handle_data(data,
						pchRcvDataFrame, iDataIdx,
						iDataIdx + length);
			iDataIdx += length;
			break;
		case MSG2AP_INST_BIG_DATA:
			handle_big_data(data, pchRcvDataFrame, &iDataIdx);
			break;
		case MSG2AP_INST_TIME_SYNC:
			data->bTimeSyncing = true;
			break;
		}
	}

	if (data->bTimeSyncing)
		data->timestamp = ts.tv_sec * 1000000000ULL + ts.tv_nsec;

	return SUCCESS;
}

static void get_dummy_sensordata(char *pchRcvDataFrame, int *iDataIdx,
	struct sensor_value *sensorsdata)
{
	ssp_dbg("not supported\n");
}

void ssp_temp_task(struct work_struct *temp)
{
	ssp_dbg("not supported\n");
}

void report_dummy_data(struct ssp_data *data, struct sensor_value *value)
{
	ssp_dbg("not supported\n");
}

void initialize_function_pointer(struct ssp_data *data)
{
	data->get_sensor_data[ACCELEROMETER_SENSOR] = get_3axis_sensordata;
	data->get_sensor_data[GYROSCOPE_SENSOR] = get_3axis_sensordata;
#ifdef CONFIG_SENSORS_SSP_ADPD142
	data->get_sensor_data[BIO_HRM_RAW] = get_hrm_raw_sensordata;
	data->get_sensor_data[BIO_HRM_RAW_FAC] = get_hrm_raw_fac_sensordata;
	data->get_sensor_data[BIO_HRM_LIB] = get_hrm_lib_sensordata;
#endif
	data->get_sensor_data[GEOMAGNETIC_UNCALIB_SENSOR] =
		get_dummy_sensordata;
	data->get_sensor_data[GEOMAGNETIC_RAW] = get_dummy_sensordata;
	data->get_sensor_data[GEOMAGNETIC_SENSOR] =
		get_dummy_sensordata;
	data->get_sensor_data[PRESSURE_SENSOR] = get_dummy_sensordata;
	data->get_sensor_data[GESTURE_SENSOR] = get_dummy_sensordata;
	data->get_sensor_data[PROXIMITY_SENSOR] = get_dummy_sensordata;
	data->get_sensor_data[PROXIMITY_RAW] = get_dummy_sensordata;
	data->get_sensor_data[LIGHT_SENSOR] = get_dummy_sensordata;
	data->get_sensor_data[TEMPERATURE_HUMIDITY_SENSOR] =
		get_dummy_sensordata;
	data->get_sensor_data[ROTATION_VECTOR] = get_dummy_sensordata;
	data->get_sensor_data[GAME_ROTATION_VECTOR] = get_dummy_sensordata;
	data->get_sensor_data[STEP_DETECTOR] = get_dummy_sensordata;
	data->get_sensor_data[SIG_MOTION_SENSOR] = get_dummy_sensordata;
	data->get_sensor_data[GYRO_UNCALIB_SENSOR] = get_dummy_sensordata;
	data->get_sensor_data[STEP_COUNTER] = get_dummy_sensordata;

	data->report_sensor_data[ACCELEROMETER_SENSOR] = report_acc_data;
	data->report_sensor_data[GYROSCOPE_SENSOR] = report_gyro_data;
#ifdef CONFIG_SENSORS_SSP_ADPD142
	data->report_sensor_data[BIO_HRM_RAW] = report_hrm_raw_data;
	data->report_sensor_data[BIO_HRM_RAW_FAC] = report_hrm_raw_fac_data;
	data->report_sensor_data[BIO_HRM_LIB] = report_hrm_lib_data;
#endif
	data->report_sensor_data[GEOMAGNETIC_UNCALIB_SENSOR] =
		report_dummy_data;
	data->report_sensor_data[GEOMAGNETIC_RAW] = report_dummy_data;
	data->report_sensor_data[GEOMAGNETIC_SENSOR] =
		report_dummy_data;
	data->report_sensor_data[PRESSURE_SENSOR] = report_dummy_data;
	data->report_sensor_data[GESTURE_SENSOR] = report_dummy_data;
	data->report_sensor_data[PROXIMITY_SENSOR] = report_dummy_data;
	data->report_sensor_data[PROXIMITY_RAW] = report_dummy_data;
	data->report_sensor_data[LIGHT_SENSOR] = report_dummy_data;
	data->report_sensor_data[TEMPERATURE_HUMIDITY_SENSOR] =
		report_dummy_data;
	data->report_sensor_data[ROTATION_VECTOR] = report_dummy_data;
	data->report_sensor_data[GAME_ROTATION_VECTOR] = report_dummy_data;
	data->report_sensor_data[STEP_DETECTOR] = report_dummy_data;
	data->report_sensor_data[SIG_MOTION_SENSOR] = report_dummy_data;
	data->report_sensor_data[GYRO_UNCALIB_SENSOR] = report_dummy_data;
	data->report_sensor_data[STEP_COUNTER] = report_dummy_data;

	data->ssp_big_task[BIG_TYPE_DUMP] = ssp_dump_task;
	data->ssp_big_task[BIG_TYPE_READ_LIB] = ssp_read_big_library_task;
	data->ssp_big_task[BIG_TYPE_VOICE_NET] = ssp_send_big_library_task;
	data->ssp_big_task[BIG_TYPE_VOICE_GRAM] = ssp_send_big_library_task;
	data->ssp_big_task[BIG_TYPE_VOICE_PCM] = ssp_pcm_dump_task;
	data->ssp_big_task[BIG_TYPE_TEMP] = ssp_temp_task;
}
