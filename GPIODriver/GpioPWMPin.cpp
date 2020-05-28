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



	GPIOPWMOutputPin::GPIOPWMOutputPin(GPIOEventPackageQueue* pGPIOEventPackageQueue, int PinNo, double InitPinValue) : GPIOPin(pGPIOEventPackageQueue) {

		char szbuf[50];
		sprintf_s(szbuf, "PWM.%02d", PinNo);
		m_PinName = szbuf;
		if (InitPinValue > 1)InitPinValue = 1;

		m_SetValue = InitPinValue;
		m_InitValue = InitPinValue;
		m_PinValue = InitPinValue;
		m_PinNumber = PinNo;
		m_Frequency = -1;
		m_Typ = GPIOTyp::PWM;
#if !GPIOCLIENT_USING
		m_PWMPin = nullptr;
#else

#endif



	};




	GPIOPWMOutputPin::~GPIOPWMOutputPin()
	{
		setOutputToInitialValue();

#if !GPIOCLIENT_USING
		if (m_PWMPin != nullptr) {
			m_PWMPin->Stop();
			delete m_PWMPin;
		}

#else

#endif


	};

	void GPIOPWMOutputPin::setFrequency(double value) {
		m_Frequency = value;

	}
	double GPIOPWMOutputPin::getFrequency()
	{
		return m_Frequency;
	}

	void GPIOPWMOutputPin::UpdatesetFrequency(Windows::Devices::Pwm::PwmController^ PwmController, double value) {
		m_Frequency = value;
#if !GPIOCLIENT_USING
		if (PwmController == nullptr) return;
		try {

			Lock();
			if (PwmController->ActualFrequency != m_Frequency) { // 40 -1000 Hz is posible
				PwmController->SetDesiredFrequency(m_Frequency); // try to match 50Hz
			}
			UnLock();

		}
		catch (Exception ^ ex)
		{
			UnLock();
			return;
		}
#else
		return;
#endif

	}



	bool GPIOPWMOutputPin::setOutputToInitialValue()
	{
#if !GPIOCLIENT_USING
		try {
			if (m_PWMPin != nullptr) {
				m_PWMPin->SetActiveDutyCyclePercentage(m_InitValue);
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

	Platform::String^ GPIOPWMOutputPin::GetGPIOPinCmd()
	{
#if !GPIOCLIENT_USING
		m_ActTimeinNanos = getActualTimeinNanos();
		double Value;
		wchar_t buffer[500];
		Platform::String^ PinName = StringFromAscIIChars((char*)m_PinName.c_str());
		if (m_PWMPin != nullptr) {

			if (m_PWMPin->IsStarted) {
				Value = m_PWMPin->GetActiveDutyCyclePercentage();
			}
			else Value = -1;
			swprintf(&buffer[0], sizeof(buffer) / sizeof(buffer[0]), L"%s = %1.1f, Frequ=%2.2f, TimeNs = %I64u;", PinName->Data(), Value, m_Frequency, (UINT64)m_ActTimeinNanos);
		}

		Platform::String^ ret = ref new Platform::String(buffer);;
		return ret;
#else
		return L"";
#endif

	}


	Platform::String^ GPIOPWMOutputPin::GetGPIOPinClientSendCmd()
	{
		wchar_t buffer[500];
		Platform::String^ PinName = StringFromAscIIChars((char*)m_PinName.c_str());
		buffer[0] = 0;
		switch (m_Typ)
		{

		case GPIOTyp::PWM:
		case GPIOTyp::PWM9685:
		{
			swprintf(&buffer[0], sizeof(buffer) / sizeof(buffer[0]), L"%s = %1.1f, Frequ=%2.2f;", PinName->Data(), m_SetValue, m_Frequency);
			break;
		}

		default:
		{
			break;
		}
		}

		Platform::String^ ret = ref new Platform::String(buffer);;
		return ret;
	}
	bool GPIOPWMOutputPin::Init(Windows::Devices::Pwm::PwmController^ PwmController, double frequency) {
#if !GPIOCLIENT_USING
		if (PwmController == nullptr) return nullptr;
		try {
			m_Frequency = frequency;
			GpioPin^ pin;
			Lock();
			if (PwmController->ActualFrequency != m_Frequency) { // 40 -1000 Hz is posible
				PwmController->SetDesiredFrequency(m_Frequency); // try to match 50Hz
			}

			int pinNo = m_PinNumber;

			auto _pin = PwmController->OpenPin(pinNo);
			if (_pin != nullptr)
			{
				_pin->SetActiveDutyCyclePercentage(m_InitValue); // 0.0-1.0 is possible
				m_PinValue = m_InitValue;
				m_SetValue = m_InitValue;
				m_PWMPin = _pin;
				//m_pGPIOEventPackageQueue->PushPacket(new GPIOEventPackage(this->m_PinNumber, this->GetGPIOPinCmd()));
				m_pGPIOEventPackageQueue->PushPacket(new GPIOEventPackage(this->m_PinNumber, this));

				UnLock();
				return true;
			}
			UnLock();
			return false;
		}
		catch (Exception ^ ex)
		{
			UnLock();
			return false;
		}
#else
		return false;
#endif


	}



	bool GPIOPWMOutputPin::doProcessing()
	{

#if !GPIOCLIENT_USING

		try {
			Lock();
			if (m_PWMPin != nullptr)
			{
				bool bPacket = false;
				if (m_SetValue >= 0) {
					if (m_SetValue > 1)m_SetValue = 1;
					if (m_SetValue < 0)m_SetValue = 0;
					if (m_PWMPin->GetActiveDutyCyclePercentage() != m_SetValue)
					{
						m_PWMPin->SetActiveDutyCyclePercentage(m_SetValue);
						if (!m_PWMPin->IsStarted) {
							m_PWMPin->Start();
						}
						bPacket = true;
					}
				}
				else
				{
					m_PWMPin->SetActiveDutyCyclePercentage(0);
					m_PWMPin->Stop();
					bPacket = true;
				}

				if (bPacket) {
					m_PinValue = m_SetValue;
				//	m_pGPIOEventPackageQueue->PushPacket(new GPIOEventPackage(this->m_PinNumber, this->GetGPIOPinCmd()));
					m_pGPIOEventPackageQueue->PushPacket(new GPIOEventPackage(this->m_PinNumber, this));
				}

			}
			UnLock();
			return true;
		}
		catch (Exception ^ ex)
		{
			UnLock();
			return false;
		}
#else
		return true;
#endif

	}


	GPIOPWMPWM9685OutputPin::GPIOPWMPWM9685OutputPin(GPIOEventPackageQueue* pGPIOEventPackageQueue, int PinNo, double InitPinValue) : GPIOPWMOutputPin(pGPIOEventPackageQueue, PinNo, InitPinValue) {


		char szbuf[50];
		sprintf_s(szbuf, "PWM9685.%02d", PinNo);
		m_PinName = szbuf;
		m_Typ = GPIOTyp::PWM9685;

	};




	GPIOPWMPWM9685OutputPin::~GPIOPWMPWM9685OutputPin()
	{


	};

}
