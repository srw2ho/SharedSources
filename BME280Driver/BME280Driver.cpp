#include "pch.h"

#include "bme280.h"
#include "BME280Driver.h"

using namespace BME280Driver;


int8_t user_i2c_read(uint8_t id, uint8_t reg_addr, uint8_t *data, uint16_t len)
{

	return -1;
}

void user_delay_ms(uint32_t period)
{
	//usleep(period * 1000);
	Sleep(period);// Sleep in ms
}

int8_t user_i2c_write(uint8_t id, uint8_t reg_addr, uint8_t *data, uint16_t len)
{

	return -1;
}

void BME280IoTDriver::print_sensor_data(struct bme280_data *comp_data)
{
#ifdef BME280_FLOAT_ENABLE
	//printf("temp %0.2f, p %0.2f, hum %0.2f\r\n", comp_data->temperature, comp_data->pressure, comp_data->humidity);
#else
	//printf("temp %ld, p %ld, hum %ld\r\n", comp_data->temperature, comp_data->pressure, comp_data->humidity);
#endif
}

int8_t BME280IoTDriver::stream_sensor_data_forced_mode()
{
	int8_t rslt;
	uint8_t settings_sel;
	struct bme280_data comp_data;

	struct bme280_dev *dev = &this->m_dev;
	
	/* Recommended mode of operation: Indoor navigation */
	dev->settings.osr_h = BME280_OVERSAMPLING_1X;
	dev->settings.osr_p = BME280_OVERSAMPLING_16X;
	dev->settings.osr_t = BME280_OVERSAMPLING_2X;
	dev->settings.filter = BME280_FILTER_COEFF_16;

	settings_sel = BME280_OSR_PRESS_SEL | BME280_OSR_TEMP_SEL | BME280_OSR_HUM_SEL | BME280_FILTER_SEL;

	rslt = bme280_set_sensor_settings(settings_sel, dev);

	//m_req_delay = bme280_cal_meas_delay(&dev->settings);
	m_req_delay = bme280_cal_meas_delay(&m_dev.settings);
	//printf("Temperature, Pressure, Humidity\r\n");
	/* Continuously stream sensor data */
	while (1) {
		rslt = bme280_set_sensor_mode(BME280_FORCED_MODE, dev);
		/* Wait for the measurement to complete and print data @25Hz */
		dev->delay_ms(m_req_delay);
		rslt = bme280_get_sensor_data(BME280_ALL, &m_comp_data, dev);
		print_sensor_data(&comp_data);
	}
	return rslt;
}

int8_t BME280IoTDriver::stream_sensor_data_normal_mode()
{
	struct bme280_dev *dev = &this->m_dev;
	int8_t rslt;
	uint8_t settings_sel;

	/* Recommended mode of operation: Indoor navigation */
	dev->settings.osr_h = BME280_OVERSAMPLING_1X;
	dev->settings.osr_p = BME280_OVERSAMPLING_16X;
	dev->settings.osr_t = BME280_OVERSAMPLING_2X;
	dev->settings.filter = BME280_FILTER_COEFF_16;
	dev->settings.standby_time = BME280_STANDBY_TIME_62_5_MS;

	settings_sel = BME280_OSR_PRESS_SEL;
	settings_sel |= BME280_OSR_TEMP_SEL;
	settings_sel |= BME280_OSR_HUM_SEL;
	settings_sel |= BME280_STANDBY_SEL;
	settings_sel |= BME280_FILTER_SEL;
	rslt = bme280_set_sensor_settings(settings_sel, dev);
	rslt = bme280_set_sensor_mode(BME280_NORMAL_MODE, dev);

//	printf("Temperature, Pressure, Humidity\r\n");
	while (1) {
		/* Delay while the sensor completes a measurement */
		dev->delay_ms(70);
		rslt = bme280_get_sensor_data(BME280_ALL, &m_comp_data, dev);
		print_sensor_data(&m_comp_data);
	}

	return rslt;
}



BME280IoTDriver::BME280IoTDriver(uint8_t dev_id)
{

	memset(&m_comp_data, 0, sizeof(m_comp_data));
	memset(&m_dev, 0, sizeof(m_dev));
	m_dev.dev_id = dev_id;
	m_readFkt =nullptr;
	m_writeFkt = nullptr;
	m_delay_ms = nullptr;
}


BME280IoTDriver::~BME280IoTDriver()
{


}




int BME280IoTDriver::ReadSensorDataIntoNormalMode() {

	int8_t rslt;

	//m_dev.delay_ms(70);
	rslt = bme280_get_sensor_data(BME280_ALL, &m_comp_data, &m_dev);

	return rslt;
}

int BME280IoTDriver::ReadSensorDataIntoForcedMode() {

	int8_t rslt;
	rslt = bme280_set_sensor_mode(BME280_FORCED_MODE, &m_dev); // only for one cycle
	/* Wait for the measurement to complete and print data @25Hz */
	m_dev.delay_ms(m_req_delay);
	rslt = bme280_get_sensor_data(BME280_ALL, &m_comp_data, &m_dev);
	
	return rslt;
}

double BME280IoTDriver::getPressure()
{

#ifdef BME280_FLOAT_ENABLE
	return m_comp_data.pressure;
#else

#ifdef BME280_64BIT_ENABLE
	return (m_comp_data.pressure / 100.0);
#else
	return (m_comp_data.pressure);
#endif


#endif /* BME280_USE_FLOATING_POINT */
}

double BME280IoTDriver::getPressurehPa()
{

#ifdef BME280_FLOAT_ENABLE
	return m_comp_data.pressure / 100.0;
#else
#ifdef BME280_64BIT_ENABLE
	return (m_comp_data.pressure / 10000.0);
#else
	return (m_comp_data.pressure / 100.0);
#endif


#endif /* BME280_USE_FLOATING_POINT */
}

double BME280IoTDriver::getTemperature()
{
#ifdef BME280_FLOAT_ENABLE
	return m_comp_data.temperature;
#else
#ifdef BME280_64BIT_ENABLE

	return m_comp_data.temperature / 100.0;

#else
	return (m_comp_data.temperature / 100.0);
#endif


#endif /* BME280_USE_FLOATING_POINT */

}
double BME280IoTDriver::getTemperatureFahrenheit()
{
	return getTemperature()*1.8 + 32;
}


double BME280IoTDriver::getHumidity()
{

#ifdef BME280_FLOAT_ENABLE
	return m_comp_data.humidity;
#else
#ifdef BME280_64BIT_ENABLE
	return (m_comp_data.humidity / 1024.0);
#else
	return (m_comp_data.humidity / 1024.0);
#endif


#endif /* BME280_USE_FLOATING_POINT */

}


int BME280IoTDriver::setForceModeSettings()
{
		int8_t rslt;
		uint8_t settings_sel;

		/* Recommended mode of operation: Indoor navigation */
		m_dev.settings.osr_h = BME280_OVERSAMPLING_1X;
		m_dev.settings.osr_p = BME280_OVERSAMPLING_16X;
		m_dev.settings.osr_t = BME280_OVERSAMPLING_2X;
		m_dev.settings.filter = BME280_FILTER_COEFF_16;

		settings_sel = BME280_OSR_PRESS_SEL | BME280_OSR_TEMP_SEL | BME280_OSR_HUM_SEL | BME280_FILTER_SEL;
		rslt = bme280_set_sensor_settings(settings_sel, &m_dev);

		m_req_delay = bme280_cal_meas_delay(&m_dev.settings);

	//	rslt = bme280_set_sensor_mode(BME280_FORCED_MODE, &m_dev) | rslt ;
		return rslt;

}

int BME280IoTDriver::setNormalModeSettings()
{
	int8_t rslt;
	uint8_t settings_sel;
	m_dev.settings.osr_h = BME280_OVERSAMPLING_1X;
	m_dev.settings.osr_p = BME280_OVERSAMPLING_16X;
	m_dev.settings.osr_t = BME280_OVERSAMPLING_2X;
	m_dev.settings.filter = BME280_FILTER_COEFF_16;
	m_dev.settings.standby_time = BME280_STANDBY_TIME_62_5_MS;

	settings_sel = BME280_OSR_PRESS_SEL;
	settings_sel |= BME280_OSR_TEMP_SEL;
	settings_sel |= BME280_OSR_HUM_SEL;
	settings_sel |= BME280_STANDBY_SEL;
	settings_sel |= BME280_FILTER_SEL;
	rslt = bme280_set_sensor_settings(settings_sel, &m_dev);
	rslt = bme280_set_sensor_mode(BME280_NORMAL_MODE, &m_dev) | rslt;;
	return rslt;

}

int BME280IoTDriver::Initialization() {

	//struct bme280_dev dev;
	int8_t rslt = BME280_OK;

//	m_dev.dev_id = BME280_I2C_ADDR_PRIM;
	m_dev.intf = BME280_I2C_INTF;

	if (m_readFkt!=nullptr)	m_dev.read = m_readFkt;
	else  m_dev.read = user_i2c_read;
	
	if (m_writeFkt!=nullptr) m_dev.write = m_writeFkt;
	else m_dev.write = user_i2c_write;

	if (m_delay_ms != nullptr) m_dev.delay_ms = m_delay_ms;
	else m_dev.delay_ms = user_delay_ms;

	rslt = bme280_init(&m_dev);

//	stream_sensor_data_forced_mode();
	return rslt;

}