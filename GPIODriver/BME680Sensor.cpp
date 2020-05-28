#include "pch.h"
#include "GPIODriver.h"
#include "GPIOPin.h"



using namespace Platform;
using namespace std;

using namespace Windows::ApplicationModel::AppService;

using namespace Windows::Devices::Pwm;
using namespace Windows::Devices::Gpio;
using namespace Windows::Foundation;
using namespace concurrency;
using namespace Windows::System::Threading;
using namespace Windows::Foundation::Collections;
using namespace  Windows::System;





namespace GPIODriver
{

	BME280Sensor::BME280Sensor(GPIOEventPackageQueue* pGPIOEventPackageQueue, int PinNo, double InitPinValue) : GPIOPin(pGPIOEventPackageQueue) {

		char szbuf[50];
		sprintf_s(szbuf, "BME280.%02d", PinNo);
		m_PinName = szbuf;
		m_InitValue = InitPinValue;
		m_PinValue = -1; // keine Messung erkannt
		m_PinNumber = PinNo;
		m_Typ = GPIOTyp::BME280;


		m_pressure = -1; /*! Compensated temperature */

		m_temperature = -1; 	/*! Compensated humidity */

		m_humidity = -1;


	};


	BME280Sensor::~BME280Sensor()
	{




	};

	Platform::String^ BME280Sensor::GetGPIOPinCmd()
	{

		m_ActTimeinNanos = getActualTimeinNanos();
		wchar_t buffer[500];
		Platform::String^ PinName = StringFromAscIIChars((char*)m_PinName.c_str());


		swprintf(&buffer[0], sizeof(buffer) / sizeof(buffer[0]), L"%s = %02.2f, TimeNs = %I64u, Temp = %02.2f, Press= %02.2f, Humid= %02.2f;", PinName->Data(), this->m_PinValue, (UINT64)m_ActTimeinNanos, this->m_temperature, this->m_pressure, this->m_humidity);

		Platform::String^ ret = ref new Platform::String(buffer);;
		return ret;

	}

	Platform::String^ BME280Sensor::GetGPIOPinClientSendCmd()
	{

		wchar_t buffer[500];
		Platform::String^ PinName = StringFromAscIIChars((char*)m_PinName.c_str());

		swprintf(&buffer[0], sizeof(buffer) / sizeof(buffer[0]), L"%s = %02d;", PinName->Data(), (int)this->m_SetValue);

		Platform::String^ ret = ref new Platform::String(buffer);;
		return ret;

	}

	bool BME280Sensor::doProcessing()
	{
		return false;
	}


	bool BME280Sensor::Init(Windows::Devices::Gpio::GpioController^ GPIOController)
	{

		return false;


	}


	BME680Sensor::BME680Sensor(GPIOEventPackageQueue* pGPIOEventPackageQueue, int PinNo, double InitPinValue) : GPIOPin(pGPIOEventPackageQueue) {

		char szbuf[50];
		sprintf_s(szbuf, "BME680.%02d", PinNo);
		m_PinName = szbuf;
		m_InitValue = InitPinValue;
		m_PinValue = -1; // keine Messung erkannt
		m_PinNumber = PinNo;
		m_Typ = GPIOTyp::BME680;


		m_iaq = -1;
		m_iaq_accuracy = -1;
		m_temperature = -1;
		m_humidity = -1;
		m_pressure = -1;
		m_raw_temperature = -1;
		m_raw_humidity = -1;
		m_gas = -1;
		m_bsec_status = -1;
		m_static_iaq = -1;
		m_co2_equivalent = -1;
		m_breath_voc_equivalent = -1;

	};



	BME680Sensor::~BME680Sensor()
	{
		

	};



	bool BME680Sensor::doProcessing()
	{
		return false;
	}


	bool BME680Sensor::Init(Windows::Devices::Gpio::GpioController^ GPIOController)
	{

		return false;


	}

}
