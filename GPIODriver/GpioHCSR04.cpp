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
	//typedef std::map<int, GPIODriver::GPIOHCSR04*> MapGpioPinGPIOHCSR04Changed;

	//MapGpioPinGPIOHCSR04Changed globGPIOOHCSR04Changed;

	GPIOHCSR04::GPIOHCSR04(GPIOEventPackageQueue* pGPIOEventPackageQueue, int PinNo, int TriggerPin, double InitPinValue) : GPIOPin(pGPIOEventPackageQueue) {

		char szbuf[50];
		sprintf_s(szbuf, "HC_SR04.%02d", PinNo);
		m_PinName = szbuf;
		m_InitValue = InitPinValue;
		m_PinValue = -1; // keine Messung erkannt
		m_PinNumber = PinNo;
		m_Typ = GPIOTyp::HC_SR04;

		m_ActMeasTime = 0;

		//m_hFinishedEvent = CreateEvent(
		//	NULL,               // default security attributes
		//	TRUE,               // manual-reset event
		//	FALSE,              // initial state is nonsignaled
		//	nullptr
		//	//TEXT("WriteEvent")  // object name
		//);

		m_TriggerPinNumber = TriggerPin;
#if !GPIOCLIENT_USING
		m_TriggerPin = nullptr;
#else

#endif

	};


	GPIOHCSR04::~GPIOHCSR04()
	{

		setOutputToInitialValue();
#if !GPIOCLIENT_USING

		if (m_TriggerPin != nullptr) {
			delete m_TriggerPin;
		}

	//	globGPIOOHCSR04Changed.erase(m_PinNumber);
	//	CloseHandle(m_hFinishedEvent);
#else

#endif


	};


	bool GPIOHCSR04::setOutputToInitialValue()
	{
#if !GPIOCLIENT_USING
		try {
			if (m_TriggerPin != nullptr) {
				// Trigger-Pin Ausgang auf Low setzen
				m_TriggerPin->Write(GpioPinValue::Low);

			}
			return true;
		}
		catch (Exception ^ ex)
		{
			return false;
		}

#else
		return false;
#endif
	}

	Platform::String^ GPIOHCSR04::GetGPIOPinCmd()
	{
		m_ActTimeinNanos = getActualTimeinNanos();

		wchar_t buffer[500];
		Platform::String^ PinName = StringFromAscIIChars((char*)m_PinName.c_str());

		swprintf(&buffer[0], sizeof(buffer) / sizeof(buffer[0]), L"%s = %06.0f, TimeNs = %I64u , TrigPin= %02d;", PinName->Data(), this->m_PinValue, (UINT64)m_ActTimeinNanos, this->m_TriggerPinNumber);

		Platform::String^ ret = ref new Platform::String(buffer);;
		return ret;

	}

	Platform::String^ GPIOHCSR04::GetGPIOPinClientSendCmd()
	{

		wchar_t buffer[500];
		Platform::String^ PinName = StringFromAscIIChars((char*)m_PinName.c_str());

		swprintf(&buffer[0], sizeof(buffer) / sizeof(buffer[0]), L"%s = %06.0f, TrigPin= %02d;", PinName->Data(), this->m_SetValue, this->m_TriggerPinNumber);

		Platform::String^ ret = ref new Platform::String(buffer);;
		return ret;

	}



	void GPIOHCSR04::setTriggerPinNumber(int pinNo)
	{
		Lock();
		m_TriggerPinNumber = pinNo;
		UnLock();
	}




	bool GPIOHCSR04::Init(Windows::Devices::Gpio::GpioController^ GPIOController)
	{
#if !GPIOCLIENT_USING
		if (GPIOController == nullptr) return false;

		GpioPin^ pin;
		GpioPin^ Echopin;
		Lock();
		GpioOpenStatus  status;
		int pinNo = m_PinNumber;
		GPIOController->TryOpenPin(pinNo, GpioSharingMode::Exclusive, &pin, &status);
		if (pin != nullptr) {
			if (status == GpioOpenStatus::PinOpened)
			{
				if (pin->IsDriveModeSupported(GpioPinDriveMode::Input)) {

					pin->SetDriveMode(GpioPinDriveMode::Input);
					m_Pin = pin;
					m_PinValue = -1; // not measured

					GPIOController->TryOpenPin(m_TriggerPinNumber, GpioSharingMode::Exclusive, &Echopin, &status);
					if (Echopin != nullptr) {
						if (status == GpioOpenStatus::PinOpened)
						{
							if (Echopin->IsDriveModeSupported(GpioPinDriveMode::Output)) {
							//	globGPIOOHCSR04Changed[pinNo] = this; // Map for fast finding object in case of ValueChanged

					//			m_valueChangedToken = m_Pin->ValueChanged += ref new Windows::Foundation::TypedEventHandler<Windows::Devices::Gpio::GpioPin ^, Windows::Devices::Gpio::GpioPinValueChangedEventArgs ^>(&OnValueHCSR04Changed);

								m_TriggerPin = Echopin;
								UnLock();
								return true;
							}
						}
						delete Echopin;
					}

				}
			}
			delete pin;
		}

		UnLock();
		return false;
#else
		return false;
#endif


	}

	//void GPIOHCSR04::setMeasuremenFinished(double MeasurementTime)
	//{
	//	this->m_PinValue = MeasurementTime;
	//	this->m_PulseTimeActive = false;
	//	::SetEvent(m_hFinishedEvent);
	//}


	//void GPIOHCSR04::OnValueChanged(LONGLONG TimeinYsec, Windows::Devices::Gpio::GpioPinValueChangedEventArgs^ args)
	//{
	//}

	bool GPIOHCSR04::doMeasurement()
	{

#if !GPIOCLIENT_USING
		bool ret = false;


		Lock();

		double MeasValue = -1;
		bool bHighFlank = false;
		LONGLONG TimeinYsec;

		m_PulseTimeActive = true;

		m_TriggerPin->Write(Windows::Devices::Gpio::GpioPinValue::High);

		HardwaitintoYsec(50); // 50 µs Hard Wait loop

		m_TriggerPin->Write(Windows::Devices::Gpio::GpioPinValue::Low);

		m_ActMeasTime = GetPerformanceCounter();


		do {
			TimeinYsec = GetPerformanceCounter(); // time in µs
			if (m_Pin->Read() == Windows::Devices::Gpio::GpioPinValue::High)
			{
				m_ActMeasTime = TimeinYsec;
				bHighFlank = true;
				break;
			}
			else if (TimeinYsec - m_ActMeasTime > 600)  // 400 µs nach 600 µs muss das Echo da sein
			{
				break;
			}


		} while (1);

		if (bHighFlank) {
			do {
				TimeinYsec = GetPerformanceCounter(); // time in µs
				if (m_Pin->Read() == Windows::Devices::Gpio::GpioPinValue::Low)
				{
					double MeasurementTime = (double)(TimeinYsec - m_ActMeasTime);
					MeasValue = MeasurementTime;
					break;
				}
				else if (TimeinYsec - m_ActMeasTime > 20000) // 20*1000 µs
				{
					break;
				}

			} while (1);

		}

		this->m_PinValue = MeasValue; // keine Messung empfangen

		m_PulseTimeActive = false;
		//m_pGPIOEventPackageQueue->PushPacket(new GPIOEventPackage(this->m_PinNumber, this->GetGPIOPinCmd()));
		m_pGPIOEventPackageQueue->PushPacket(new GPIOEventPackage(this->m_PinNumber, this));

		ret = (this->m_PinValue != -1);
		UnLock();

		return ret;
#else
		return false;
#endif


	}


	bool GPIOHCSR04::doProcessing()
	{
		return doMeasurement();;
	}


	//void GPIODriver::GPIOHCSR04::OnValueHCSR04Changed(Windows::Devices::Gpio::GpioPin^ sender, Windows::Devices::Gpio::GpioPinValueChangedEventArgs^ args)
	//{

	//	//typedef std::map<int, GPIODriver::GPIOHCSR04*> MapGpioPinGPIOHCSR04Changed;
	//	LONGLONG actTime = GetPerformanceCounter(); // time in µs

	//	MapGpioPinGPIOHCSR04Changed::const_iterator it;

	//	it = globGPIOOHCSR04Changed.find(sender->PinNumber); // Object zum zugehörigen InputPin suchen

	//	if (it != globGPIOOHCSR04Changed.end())
	//	{
	//		GPIODriver::GPIOHCSR04* pGPIOHCSR04 = it->second;

	//		pGPIOHCSR04->OnValueChanged(actTime, args);

	//	}


	//}


}
