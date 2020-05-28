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


	ADCMT3620::ADCMT3620(GPIOEventPackageQueue* pGPIOEventPackageQueue, int PinNo, double InitPinValue) : GPIOPin(pGPIOEventPackageQueue) {

		char szbuf[50];
		sprintf_s(szbuf, "ADC_MT3620.%02d", PinNo);
		m_PinName = szbuf;
		m_InitValue = InitPinValue;
		m_PinValue = -1; // keine Messung erkannt
		m_PinNumber = PinNo;
		m_Typ = GPIOTyp::ADC_MT3620;


	};



	ADCMT3620::~ADCMT3620()
	{


	};



	bool ADCMT3620::doProcessing()
	{
		return false;
	}


	bool ADCMT3620::Init(Windows::Devices::Gpio::GpioController^ GPIOController)
	{

		return false;


	}

}
