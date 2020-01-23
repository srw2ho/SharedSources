#include "pch.h"
#include "GPIODriver.h"
#include "GPIOPin.h"


#pragma once

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


#if !GPIOCLIENT_USING

using namespace Microsoft::IoT::Lightning::Providers;

#else

#endif



#define PWM_Frequency		50

const UINT64 nano100SecInSec = (UINT64)10000000 * 1;


namespace GPIODriver
{
	typedef std::map<int, GPIODriver::GPIOInputPin*> MapGpioPinInputChanged;

	typedef std::map<int, GPIODriver::GPIOOutputPin*> MapGpioPinOutputChanged;

	typedef std::map<int, GPIODriver::GPIOHCSR04*> MapGpioPinGPIOHCSR04Changed;

	MapGpioPinInputChanged globGPIOPinInputChanged;

	MapGpioPinOutputChanged globGPIOPinOutputChanged;

	MapGpioPinGPIOHCSR04Changed globGPIOOHCSR04Changed;


	GPIOPin::GPIOPin(GPIOEventPackageQueue* pGPIOEventPackageQueue) {
		InitializeCriticalSection(&m_CritLock);


#if !GPIOCLIENT_USING
		m_Pin = nullptr;
#else

#endif

		m_PinName.clear();

		m_PinValue = 0;
		m_SetValue = 0;
		m_Typ = GPIOTyp::undef;
		m_PinNumber = -1;
		m_PulseTimeinms = 0;
		m_PulseTimeActive = false;
		m_ActMeasTime = 0;
		m_ActTimeinNanos = -1;// Systemzeit
		m_Timer = nullptr;
		m_pGPIOEventPackageQueue = pGPIOEventPackageQueue;
		m_ActivateProcessing = false;
		m_ActivateOutputProcessing = false;

	};



	GPIOPin::~GPIOPin()
	{
		if (m_Timer != nullptr)
		{
			m_Timer->Cancel();
			m_Timer = nullptr;
		}

#if !GPIOCLIENT_USING
		DiableValueChanged();
		delete m_Pin;
		m_Pin = nullptr;

#else

#endif
		DeleteCriticalSection(&m_CritLock);

	};



	void GPIOPin::DiableValueChanged()
	{
#if !GPIOCLIENT_USING
		Lock();
		if (m_Pin != nullptr) {
			if (m_valueChangedToken.Value > 0) {
				m_Pin->ValueChanged -= m_valueChangedToken;
				m_valueChangedToken.Value = 0;
			}
		}

		UnLock();
#else

#endif




	}


	double GPIOPin::getPinValue()
	{
		Lock();
		double ret = m_PinValue;
		UnLock();
		return ret;
	}

	double GPIOPin::getSetValue()
	{
		Lock();
		double ret = m_SetValue;
		UnLock();
		return ret;
	}

	int GPIOPin::getPinNumber()
	{
		Lock();
		int ret = m_PinNumber;
		UnLock();
		return ret;
	}

	bool GPIOPin::getActivateProcessing()
	{
		Lock();
		bool ret = m_ActivateProcessing;
		UnLock();
		return ret;
	}

	bool GPIOPin::getActivateOutputProcessing()
	{
		Lock();
		bool ret = m_ActivateOutputProcessing;
		UnLock();
		return ret;
	}


	GPIODriver::GPIOTyp	GPIOPin::getGPIOTyp()
	{
		Lock();
		GPIOTyp ret = m_Typ;
		UnLock();
		return ret;

	};

	UINT64 GPIOPin::getTimeinNanos() {
		Lock();
		UINT64 ret = m_ActTimeinNanos;
		UnLock();
		return ret;
	}



	void GPIOPin::setSetValue(double Value)
	{
		Lock();
		m_SetValue = Value;
		UnLock();

	}
	void GPIOPin::setPinValue(double Value)
	{
		Lock();
		m_PinValue = Value;
		UnLock();

	}


	void GPIOPin::setPulseTimeinms(double Value)
	{
		Lock();
		m_PulseTimeinms = Value;
		UnLock();

	}
	void GPIOPin::setActivateProcessing(bool Activate) {
		Lock();
		m_ActivateProcessing = Activate;
		UnLock();
	}

	void GPIOPin::setActivateOutputProcessing(bool Activate) {
		Lock();
		m_ActivateOutputProcessing = Activate;
		UnLock();
	}



	void GPIOPin::setTimeinNanos(UINT64 nanos)
	{
		 Lock();
		 m_ActTimeinNanos = nanos;
		 UnLock();
	}


	Platform::String^ GPIOPin::GetGPIOPinCmd()
	{



		m_ActTimeinNanos = getActualTimeinNanos();
		wchar_t buffer[500];
		Platform::String^ PinName = StringFromAscIIChars((char*)m_PinName.c_str());
		double Value;

		switch (m_Typ)
		{
		case GPIOTyp::inputShutdown:
		case GPIOTyp::input:
		{
			Value = (int)m_PinValue;
			swprintf(&buffer[0], sizeof(buffer) / sizeof(buffer[0]), L"%s = %01d, TimeNs = %I64u;", PinName->Data(),(int)Value, (UINT64) m_ActTimeinNanos);
			break;
		}
		case GPIOTyp::output:
		{
#if !GPIOCLIENT_USING
			Value = (m_Pin->Read() == Windows::Devices::Gpio::GpioPinValue::High) ? 1 : 0;
#else
			Value = (int)m_PinValue;
#endif

			if (m_PulseTimeinms > 0) {
				swprintf(&buffer[0], sizeof(buffer) / sizeof(buffer[0]), L"%s = %01d, TimeNs = %I64u, Pulsetime = %d;", PinName->Data(), (int)Value, (UINT64) m_ActTimeinNanos,(int)m_PulseTimeinms);
			}
			else
			{
				swprintf(&buffer[0], sizeof(buffer) / sizeof(buffer[0]), L"%s = %01d, TimeNs = %I64u;", PinName->Data(), (int)Value, (UINT64) m_ActTimeinNanos);
			}
			break;
		}
		case GPIOTyp::PWM:
		{
			break;
		}
		case GPIOTyp::HC_SR04:
		{
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

	Platform::String^ GPIOPin::GetGPIOPinClientSendCmd()
	{
		wchar_t buffer[500];
		Platform::String^ PinName = StringFromAscIIChars((char*)m_PinName.c_str());
		buffer[0] = 0;
		double Value;

		switch (m_Typ)
		{
		case GPIOTyp::inputShutdown:
		case GPIOTyp::input:
		{
			Value = (int)m_SetValue;
			swprintf(&buffer[0], sizeof(buffer) / sizeof(buffer[0]), L"%s = %01d;", PinName->Data(), (int)Value);
			break;
			break;
		}
		case GPIOTyp::output:
		{
			Value = (int)m_SetValue;

			if (m_PulseTimeinms > 0) {
				swprintf(&buffer[0], sizeof(buffer) / sizeof(buffer[0]), L"%s = %01d , Pulsetime = %d;", PinName->Data(), (int)Value, (int)m_PulseTimeinms);
			}
			else
			{
				swprintf(&buffer[0], sizeof(buffer) / sizeof(buffer[0]), L"%s = %01d;", PinName->Data(), (int)Value);

			}
			break;
		}
		case GPIOTyp::PWM:
		case GPIOTyp::PWM9685:
		{
			swprintf(&buffer[0], sizeof(buffer) / sizeof(buffer[0]), L"%s = %1.1f;", PinName->Data(), m_SetValue);
			break;
		}
		case GPIOTyp::HC_SR04:
		{

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

	void GPIOPin::UnLock() {

		LeaveCriticalSection(&m_CritLock);
	}

	void GPIOPin::Lock() {
		EnterCriticalSection(&m_CritLock);
	}



	bool GPIOPin::setOutputToInitialValue()
	{

#if !GPIOCLIENT_USING
		try {
			if (m_Pin != nullptr) {
				if (m_Pin->GetDriveMode() == GpioPinDriveMode::Output) {
					m_Pin->Write((m_InitValue > 0) ? GpioPinValue::High : GpioPinValue::Low); // Ausgabe der Pins
				}
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

	bool GPIOPin::doClientProcessing()
	{
#if !GPIOCLIENT_USING
		return false;
#else
		m_pGPIOEventPackageQueue->PushPacket(new GPIOEventPackage(this->m_PinNumber, "", this));
		return true;
#endif

	}

	bool GPIOPin::doProcessing()
	{
#if !GPIOCLIENT_USING
		try {
			Lock();
			if (m_Pin != nullptr) {
				if (m_Pin->GetDriveMode() == GpioPinDriveMode::Output) {
					bool bsetValue = false;
					if (!m_PulseTimeActive) // forded reset = -1
					{
						double Value;
						if (m_PulseTimeinms > 0)
						{
							Value = !m_SetValue;
						}
						else
						{
							Value = (m_Pin->Read() == Windows::Devices::Gpio::GpioPinValue::High) ? 1 : 0;

						}
						if (Value != m_SetValue)
						{
							m_Pin->Write((m_SetValue > 0) ? GpioPinValue::High : GpioPinValue::Low); // Ausgabe der Pins
						}
					}

				}
				else if (m_Pin->GetDriveMode() == GpioPinDriveMode::Input) {
					/*
					if (m_Typ == GPIODriver::GPIOTyp::inputShutdown)
					{
						if (m_SetValue != m_PinValue) {

						}
					}
					*/
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
		return false;
#endif

	}





	GPIOInputPin::~GPIOInputPin()
	{
		globGPIOPinInputChanged.erase(m_PinNumber);
	};

	GPIOInputPin::GPIOInputPin(GPIOEventPackageQueue* pGPIOEventPackageQueue, int PinNo, double InitPinValue) : GPIOPin(pGPIOEventPackageQueue) {

		char szbuf[50];
		sprintf_s(szbuf, "GPI.%02d", PinNo);
		m_PinName = szbuf;
		if (InitPinValue > 1)InitPinValue = 1;

		m_InitValue = InitPinValue;
		m_PinValue = InitPinValue;
		m_PinNumber = PinNo;
		m_Typ = GPIOTyp::input;

	};


	bool GPIOInputPin::Init(Windows::Devices::Gpio::GpioController ^ GPIOController)
	{

#if !GPIOCLIENT_USING
		if (GPIOController == nullptr) return false;

		GpioPin^ pin;
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
					m_PinValue = (pin->Read() == Windows::Devices::Gpio::GpioPinValue::High) ? 1 : 0;
					m_InitValue = m_PinValue;

					globGPIOPinInputChanged[pinNo] = this;

					TimeSpan interval;
					long Msec = nano100SecInSec / 1000; // 1Sec / 1000 = 1msec
					interval.Duration = 50 * Msec; // 50 msec: Entprellzeit
					interval.Duration = interval.Duration;
					pin->DebounceTimeout = interval;

					m_valueChangedToken = m_Pin->ValueChanged += ref new Windows::Foundation::TypedEventHandler<Windows::Devices::Gpio::GpioPin ^, Windows::Devices::Gpio::GpioPinValueChangedEventArgs ^>(&OnValueInputChanged);


					UnLock();
					return true;
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



	Platform::String^ GPIOInputPin::GetGPIOPinCmd()
	{

		return  GPIOPin::GetGPIOPinCmd();

	}



	void GPIOInputPin::OnValueChanged(LONGLONG TimeinYsec, Windows::Devices::Gpio::GpioPinValueChangedEventArgs ^args)
	{

#if !GPIOCLIENT_USING
		Lock();

		bool risingEdge;
		if (args->Edge == GpioPinEdge::RisingEdge) {
			risingEdge = true;
		}
		else if (args->Edge == GpioPinEdge::FallingEdge) {
			risingEdge = false;
		}


		m_PinValue = (m_Pin->Read() == Windows::Devices::Gpio::GpioPinValue::High) ? 1 : 0;

		//	pPin->m_PinValue = (risingEdge ==true) ? 1 : 0;
		if ((m_PinValue == 0) && !risingEdge)
		{
			m_pGPIOEventPackageQueue->PushPacket(new GPIOEventPackage(this->m_PinNumber, this->GetGPIOPinCmd()));
			//	this->InputStateChanged(this, pPin->GetGPIOPinCmd());
		}
		else if ((m_PinValue == 1) && risingEdge)
		{
			m_pGPIOEventPackageQueue->PushPacket(new GPIOEventPackage(this->m_PinNumber, this->GetGPIOPinCmd()));

			//this->InputStateChanged(this, pPin->GetGPIOPinCmd());
		}

		UnLock();


#else

#endif

	}





	GPIOInputPinShutDown::~GPIOInputPinShutDown()
	{
		globGPIOPinInputChanged.erase(m_PinNumber);
	};

	GPIOInputPinShutDown::GPIOInputPinShutDown(GPIOEventPackageQueue* pGPIOEventPackageQueue, int PinNo, double InitPinValue) : GPIOInputPin(pGPIOEventPackageQueue, PinNo, InitPinValue) {



		char szbuf[50];
		sprintf_s(szbuf, "GPIShutDown.%02d", PinNo);
		m_PinName = szbuf;
		if (InitPinValue > 1)InitPinValue = 1;

		m_InitValue = InitPinValue;
		m_PinNumber = PinNo;
		m_Typ = GPIOTyp::inputShutdown;
		m_ActMeasTime = 0;
	};

	bool GPIOInputPinShutDown::doProcessing()
	{
#if !GPIOCLIENT_USING
		try {
			Lock();
			if (m_Pin != nullptr) {
				
				if (m_Pin->GetDriveMode() == GpioPinDriveMode::Input) {
					Windows::Foundation::TimeSpan TimeSpan;
					TimeSpan.Duration = 1; // 1 Sek, damit noch gemeldet werden kann
					if (m_SetValue == 2) { // Do Restart
	
						m_PinValue = m_SetValue;
						m_pGPIOEventPackageQueue->PushPacket(new GPIOEventPackage(this->m_PinNumber, this->GetGPIOPinCmd()));
						ShutdownManager::BeginShutdown(ShutdownKind::Restart, TimeSpan);

					}
					else if (m_SetValue == 3) { // Do ShutDown
					 // Ab 7 Sekunden herunterfahren
						m_PinValue = m_SetValue;
						m_pGPIOEventPackageQueue->PushPacket(new GPIOEventPackage(this->m_PinNumber, this->GetGPIOPinCmd()));
						ShutdownManager::BeginShutdown(ShutdownKind::Shutdown, TimeSpan);
					}

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
		return false;
#endif

	}

	void GPIOInputPinShutDown::OnValueChanged(LONGLONG TimeinYsec, Windows::Devices::Gpio::GpioPinValueChangedEventArgs ^args)
	{

#if !GPIOCLIENT_USING
		Lock();

		bool risingEdge;
		if (args->Edge == GpioPinEdge::RisingEdge) {
			risingEdge = true;
		}
		else if (args->Edge == GpioPinEdge::FallingEdge) {
			risingEdge = false;
		}
	
		if (m_SetValue <= 1) // not yet applied
		{

			m_PinValue = (m_Pin->Read() == Windows::Devices::Gpio::GpioPinValue::High) ? 1 : 0;

			bool doStartTime = false;
			bool doEndTime = false;

			if (m_InitValue == 1) {
				if ((m_PinValue == 0) && !risingEdge)
				{
					doStartTime = true;
				}
				else if ((m_PinValue == 1) && risingEdge)
				{
					doEndTime = true;
				}
			}
			else
				if (m_InitValue == 0)
				{
					if ((m_PinValue == 0) && !risingEdge)
					{
						doEndTime = true;
					}
					else if ((m_PinValue == 1) && risingEdge)
					{
						doStartTime = true;
					}
				}


			if (doStartTime && m_ActMeasTime == 0) {
				m_ActMeasTime = getActualTimeinNanos();
			}
			else if (doEndTime && (m_ActMeasTime != 0)) {

				UINT64 deltaTime = getActualTimeinNanos() - m_ActMeasTime;
				Windows::Foundation::TimeSpan TimeSpan;
				TimeSpan.Duration = 1; // 1 Sek, damit noch gemeldet werden kann

				if ((deltaTime > 3 * nano100SecInSec) && (deltaTime < 7 * nano100SecInSec))
				{//  Zeit von 3 Sek bis 7 Sek. -> neu Starten
					m_PinValue = 2; // Restart
					m_pGPIOEventPackageQueue->PushPacket(new GPIOEventPackage(this->m_PinNumber, this->GetGPIOPinCmd()));
					ShutdownManager::BeginShutdown(ShutdownKind::Restart, TimeSpan);
				}
				else if ((deltaTime >= 7 * nano100SecInSec))
				{ // Ab 7 Sekunden herunterfahren
					m_PinValue = 3; // ShutDown
					m_pGPIOEventPackageQueue->PushPacket(new GPIOEventPackage(this->m_PinNumber, this->GetGPIOPinCmd()));
					ShutdownManager::BeginShutdown(ShutdownKind::Shutdown, TimeSpan);
				}

				m_SetValue = m_PinValue; // only once time applied
	

				m_ActMeasTime = 0;
			}
		}
		UnLock();


#else

#endif

	}





	GPIOOutputPin::GPIOOutputPin(GPIOEventPackageQueue* pGPIOEventPackageQueue, int PinNo, double InitPinValue) : GPIOPin(pGPIOEventPackageQueue) {

		char szbuf[50];
		sprintf_s(szbuf, "GPO.%02d", PinNo);
		m_PinName = szbuf;
		if (InitPinValue > 1)InitPinValue = 1;
		m_SetValue = InitPinValue;
		m_InitValue = InitPinValue;
		m_PinValue = InitPinValue;
		m_PinNumber = PinNo;
		m_Typ = GPIOTyp::output;

	};


	GPIOOutputPin::~GPIOOutputPin()
	{

		this->setOutputToInitialValue();

		globGPIOPinOutputChanged.erase(m_PinNumber);
	};

	Platform::String^ GPIOOutputPin::GetGPIOPinCmd()
	{
		return  GPIOPin::GetGPIOPinCmd();
	}

	bool GPIOOutputPin::Init(Windows::Devices::Gpio::GpioController ^ GPIOController)
	{
#if !GPIOCLIENT_USING

		if (GPIOController == nullptr) return false;

		GpioPin^ pin;
		Lock();
		GpioOpenStatus  status;
		int pinNo = m_PinNumber;
		GPIOController->TryOpenPin(pinNo, GpioSharingMode::Exclusive, &pin, &status);
		if (pin != nullptr) {
			if (status == GpioOpenStatus::PinOpened)
			{

				if (pin->IsDriveModeSupported(GpioPinDriveMode::Output)) {
					pin->Write((m_InitValue > 0) ? GpioPinValue::High : GpioPinValue::Low); // Ausgabe der Pins
					pin->SetDriveMode(GpioPinDriveMode::Output);
					m_PinValue = m_InitValue;
					m_Pin = pin;
					globGPIOPinOutputChanged[pinNo] = this; // Map for fast finding object in case of ValueChanged

					m_valueChangedToken = m_Pin->ValueChanged += ref new Windows::Foundation::TypedEventHandler<Windows::Devices::Gpio::GpioPin ^, Windows::Devices::Gpio::GpioPinValueChangedEventArgs ^>(&OnValueOutputChanged);

					UnLock();
					return true;
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

	void GPIOOutputPin::OnValueChanged(LONGLONG TimeinYsec, Windows::Devices::Gpio::GpioPinValueChangedEventArgs ^args)
	{
#if !GPIOCLIENT_USING

		Lock();


		bool risingEdge;
		bool doEdge = false;
		if ((args->Edge == GpioPinEdge::RisingEdge) && m_SetValue > 0) { // von 0 --> 1
			risingEdge = true;
			doEdge = true;
		}
		else if ((args->Edge == GpioPinEdge::FallingEdge) && m_SetValue == 0) { // von 1--> 0
			risingEdge = false;
			doEdge = true;
		}

		if (doEdge)
		{ // bei posiiver Flanke

			if ((m_PulseTimeinms > 0) && !m_PulseTimeActive)
			{	// Pulse-Time
				m_PulseTimeActive = true;
				Windows::Foundation::TimeSpan interval;
				interval.Duration = (long long)(m_PulseTimeinms / 1000) * nano100SecInSec;

				m_PulseTimeinms = 0;

				TimerElapsedHandler ^handler = ref new TimerElapsedHandler([risingEdge, this](ThreadPoolTimer ^timer) {
					Lock();
					if (risingEdge) m_Pin->Write(GpioPinValue::Low); // Ausgabe des Pins auf Low
					else m_Pin->Write(GpioPinValue::High); // Ausgabe des Pins auf Low
					m_Timer->Cancel();
					m_Timer = nullptr;
					m_PulseTimeActive = false;
					//			this->OutputStateChanged(this, pPin->GetGPIOPinCmd()); //State Event
					UnLock();
				});
				m_Timer = ThreadPoolTimer::CreateTimer(handler, interval);// einmaliger Erzeugen eines Timers

			}

		}

		m_pGPIOEventPackageQueue->PushPacket(new GPIOEventPackage(this->m_PinNumber, this->GetGPIOPinCmd()));

		//	this->OutputStateChanged(this, pPin->GetGPIOPinCmd());

		UnLock();
#else

#endif

	}


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
		if (PwmController == nullptr) return ;
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
			return ;
		}
#else
		return ;
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
			swprintf(&buffer[0], sizeof(buffer) / sizeof(buffer[0]), L"%s = %1.1f, Frequ=%2.2f, TimeNs = %I64u;", PinName->Data(), Value, m_Frequency,(UINT64) m_ActTimeinNanos);
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
				m_pGPIOEventPackageQueue->PushPacket(new GPIOEventPackage(this->m_PinNumber, this->GetGPIOPinCmd()));

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
					m_pGPIOEventPackageQueue->PushPacket(new GPIOEventPackage(this->m_PinNumber, this->GetGPIOPinCmd()));
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


	GPIOPWMPWM9685OutputPin::GPIOPWMPWM9685OutputPin (GPIOEventPackageQueue* pGPIOEventPackageQueue, int PinNo, double InitPinValue) : GPIOPWMOutputPin(pGPIOEventPackageQueue,PinNo, InitPinValue) {


		char szbuf[50];
		sprintf_s(szbuf, "PWM9685.%02d", PinNo);
		m_PinName = szbuf;
		m_Typ = GPIOTyp::PWM9685;

	};




	GPIOPWMPWM9685OutputPin::~GPIOPWMPWM9685OutputPin()
	{


	};




	GPIOHCSR04::GPIOHCSR04(GPIOEventPackageQueue* pGPIOEventPackageQueue, int PinNo, int TriggerPin, double InitPinValue) : GPIOPin(pGPIOEventPackageQueue) {

		char szbuf[50];
		sprintf_s(szbuf, "HC_SR04.%02d", PinNo);
		m_PinName = szbuf;
		m_InitValue = InitPinValue;
		m_PinValue = -1; // keine Messung erkannt
		m_PinNumber = PinNo;
		m_Typ = GPIOTyp::HC_SR04;

		m_ActMeasTime = 0;

		m_hFinishedEvent = CreateEvent(
			NULL,               // default security attributes
			TRUE,               // manual-reset event
			FALSE,              // initial state is nonsignaled
			nullptr
			//TEXT("WriteEvent")  // object name
		);

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

		globGPIOOHCSR04Changed.erase(m_PinNumber);
		CloseHandle(m_hFinishedEvent);
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

		swprintf(&buffer[0], sizeof(buffer) / sizeof(buffer[0]), L"%s = %06.0f, TimeNs = %I64u , TrigPin= %02d;", PinName->Data(), this->m_PinValue, (UINT64) m_ActTimeinNanos, this->m_TriggerPinNumber);

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




	bool GPIOHCSR04::Init(Windows::Devices::Gpio::GpioController ^ GPIOController)
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
								globGPIOOHCSR04Changed[pinNo] = this; // Map for fast finding object in case of ValueChanged

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

	void GPIOHCSR04::setMeasuremenFinished(double MeasurementTime)
	{
		this->m_PinValue = MeasurementTime;
		this->m_PulseTimeActive = false;
		::SetEvent(m_hFinishedEvent);
	}


	void GPIOHCSR04::OnValueChanged(LONGLONG TimeinYsec, Windows::Devices::Gpio::GpioPinValueChangedEventArgs ^args)
	{
	}

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
			else if(TimeinYsec - m_ActMeasTime > 600)  // 400 µs nach 600 µs muss das Echo da sein
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
		m_pGPIOEventPackageQueue->PushPacket(new GPIOEventPackage(this->m_PinNumber, this->GetGPIOPinCmd()));

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

		
		swprintf(&buffer[0], sizeof(buffer) / sizeof(buffer[0]), L"%s = %02.2f, TimeNs = %I64u, Temp = %02.2f, Press= %02.2f, Humid= %02.2f;", PinName->Data(), this->m_PinValue, (UINT64) m_ActTimeinNanos, this->m_temperature, this->m_pressure, this->m_humidity);

		Platform::String^ ret = ref new Platform::String(buffer);;
		return ret;

	}

	Platform::String^ BME280Sensor::GetGPIOPinClientSendCmd()
	{

		wchar_t buffer[500];
		Platform::String^ PinName = StringFromAscIIChars((char*)m_PinName.c_str());

		swprintf(&buffer[0], sizeof(buffer) / sizeof(buffer[0]), L"%s = %02d;", PinName->Data(), (int) this->m_SetValue);

		Platform::String^ ret = ref new Platform::String(buffer);;
		return ret;

	}

	bool BME280Sensor::doProcessing()
	{
		return false;
	}


	bool BME280Sensor::Init(Windows::Devices::Gpio::GpioController ^ GPIOController)
	{

		return false;


	}



void GPIODriver::GPIOInputPin::OnValueInputChanged(Windows::Devices::Gpio::GpioPin ^sender, Windows::Devices::Gpio::GpioPinValueChangedEventArgs ^args)
{

	LONGLONG actTime = GetPerformanceCounter(); // time in µs

	MapGpioPinInputChanged::const_iterator it;

	it = globGPIOPinInputChanged.find(sender->PinNumber); // Object zum zugehörigen InputPin suchen

	if (it != globGPIOPinInputChanged.end())
	{
		GPIODriver::GPIOInputPin* pGPIOInputPin =  it->second;;

		pGPIOInputPin->OnValueChanged(actTime, args);

		

	}

}




void GPIODriver::GPIOOutputPin::OnValueOutputChanged(Windows::Devices::Gpio::GpioPin ^sender, Windows::Devices::Gpio::GpioPinValueChangedEventArgs ^args)
{

	LONGLONG actTime = GetPerformanceCounter(); // time in µs

	MapGpioPinOutputChanged::const_iterator it;

	it = globGPIOPinOutputChanged.find(sender->PinNumber); // Object zum zugehörigen InputPin suchen

	if (it != globGPIOPinOutputChanged.end())
	{
		GPIODriver::GPIOOutputPin* pGPIOOutputPin = it->second;;

		pGPIOOutputPin->OnValueChanged(actTime,args);


	}

}


void GPIODriver::GPIOHCSR04::OnValueHCSR04Changed(Windows::Devices::Gpio::GpioPin ^sender, Windows::Devices::Gpio::GpioPinValueChangedEventArgs ^args)
{

	//typedef std::map<int, GPIODriver::GPIOHCSR04*> MapGpioPinGPIOHCSR04Changed;
	LONGLONG actTime = GetPerformanceCounter(); // time in µs

	MapGpioPinGPIOHCSR04Changed::const_iterator it;

	it = globGPIOOHCSR04Changed.find(sender->PinNumber); // Object zum zugehörigen InputPin suchen

	if (it != globGPIOOHCSR04Changed.end())
	{
		GPIODriver::GPIOHCSR04* pGPIOHCSR04 = it->second;

		pGPIOHCSR04->OnValueChanged(actTime, args);

	}


}

}