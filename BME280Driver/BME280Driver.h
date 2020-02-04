#pragma once
#ifndef BME280IODRIVER_H_
#define BME280IODRIVER_H_

#include "BME280DriverExports.h"
#include "bme280.h"

namespace BME280Driver
{
	class BME280_EXPORT BME280IoTDriver
	{
		struct bme280_dev m_dev;
		struct bme280_data m_comp_data;

	
		bme280_com_fptr_t m_readFkt;	// callback funktion fo winIoT-Driver
		bme280_com_fptr_t m_writeFkt;	// callback funktion fo winIoT-Driver

		bme280_delay_fptr_t m_delay_ms;	// callback funktion fo winIoT-delay-Function

		uint32_t m_req_delay;
	public:
		BME280IoTDriver(uint8_t dev_id);
		virtual ~BME280IoTDriver();
		void setReadFkt(bme280_com_fptr_t readFkt) { m_readFkt = readFkt; };
		void setWriteFkt(bme280_com_fptr_t writeFkt) { m_writeFkt = writeFkt; };
		void setDelayFkt(bme280_delay_fptr_t delayFkt) { m_delay_ms = delayFkt; };

		int Initialization(); // setReadFkt,setWriteFkt must be set before Initialization can called
		int setForceModeSettings();
		int setNormalModeSettings();
		int ReadSensorDataIntoForcedMode();
		int ReadSensorDataIntoNormalMode();
		double getPressure();		// into Pascal
		double getPressurehPa();		// into ha Pascal
		double getTemperature();	// into C
		double getTemperatureFahrenheit(); // into Fahrenheit
		double getHumidity();		// into %
		int8_t getDeviceId() { return m_dev.dev_id; };
		void setDeviceId(int8_t addr) {	m_dev.dev_id = addr;};
		int8_t stream_sensor_data_normal_mode();
		int8_t stream_sensor_data_forced_mode();



	
	protected:
		void BME280IoTDriver::print_sensor_data(struct bme280_data *comp_data);
	};
}
#endif /**/