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





	PWMMT3620::PWMMT3620(GPIOEventPackageQueue* pGPIOEventPackageQueue, unsigned int PWMController, unsigned int channelNo, unsigned int Polarity, unsigned int fullCycleNs, unsigned int dutyCycleNs) : GPIOPin(pGPIOEventPackageQueue) {

		char szbuf[50];
		sprintf_s(szbuf, "PWM_MT3620.%02d", PWMController);
		m_PinName = szbuf;
		m_InitValue = 0;
		m_PinValue = -1; // keine Messung erkannt
		m_PinNumber = PWMController;
		m_Typ = GPIOTyp::PWM_MT3620;


		this->m_ChannelNos = channelNo;


		this->m_PWMChannels = (PWMChannel*)malloc(sizeof(PWMChannel) * channelNo);

		PWMChannel* pwmstate = &this->m_PWMChannels[0];

		for (size_t i = 0; i < this->m_ChannelNos; i++) {
			pwmstate->m_UserPWMState.m_fullCycleNs = fullCycleNs;
			pwmstate->m_UserPWMState.m_dutyCycleNs = dutyCycleNs;
			pwmstate->m_UserPWMState.m_enabled = true;
			pwmstate->m_UserPWMState.m_polarity = Polarity;
			pwmstate++;
		}


	};



	PWMMT3620::~PWMMT3620()
	{
		if (this->m_PWMChannels != nullptr) {
			free(this->m_PWMChannels);
			this->m_PWMChannels = nullptr;
		}


	};



	bool PWMMT3620::setfullCycleNs(unsigned int chan, unsigned int fullCycleNs) {
		Lock();

		bool retval = false;
		if (chan < this->m_ChannelNos) {
			PWMChannel* pwmstate = &this->m_PWMChannels[chan];
			pwmstate->m_UserPWMState.m_fullCycleNs = fullCycleNs;
			retval = true;
		}

		UnLock();
		return retval;

	}
	unsigned int PWMMT3620::getfullCycleNs(unsigned int chan) {

		Lock();

		unsigned int fullCycleNs = 0;

		if (chan < this->m_ChannelNos) {
			PWMChannel* pwmstate = &this->m_PWMChannels[chan];
			fullCycleNs = pwmstate->m_UserPWMState.m_fullCycleNs;
		}

		UnLock();
		return fullCycleNs;

	}

	bool PWMMT3620::setdutyCycleNs(unsigned int chan, unsigned int dutyCycleNs) {
		Lock();

		bool retval = false;
		if (chan < this->m_ChannelNos) {
			PWMChannel* pwmstate = &this->m_PWMChannels[chan];
			pwmstate->m_UserPWMState.m_dutyCycleNs = dutyCycleNs;
			retval = true;
		}
		UnLock();
		return retval;

	}

	unsigned int PWMMT3620::getdutyCycleNs(unsigned int chan) {
		Lock();

		unsigned int fullCycleNs = 0;

		if (chan < this->m_ChannelNos) {
			PWMChannel* pwmstate = &this->m_PWMChannels[chan];
			fullCycleNs = pwmstate->m_UserPWMState.m_dutyCycleNs;
		}

		UnLock();
		return fullCycleNs;

	}

	bool PWMMT3620::setenable(unsigned int chan, bool enable) {
		Lock();

		bool retval = false;
		if (chan < this->m_ChannelNos) {
			PWMChannel* pwmstate = &this->m_PWMChannels[chan];
			pwmstate->m_UserPWMState.m_enabled = enable;
			retval = true;
		}

		UnLock();
		return retval;

	}

	bool PWMMT3620::getenable(unsigned int chan) {
		Lock();

		bool retval = false;
		if (chan < this->m_ChannelNos) {
			PWMChannel* pwmstate = &this->m_PWMChannels[chan];
			retval = pwmstate->m_UserPWMState.m_enabled;
			retval = true;
		}

		UnLock();
		return retval;
	}

	unsigned int PWMMT3620::getChannelNos() {

		Lock();

		unsigned int retval = this->m_ChannelNos;

		UnLock();
		return retval;
	}

	bool PWMMT3620::doProcessing()
	{
		return false;
	}


	bool PWMMT3620::Init(Windows::Devices::Gpio::GpioController^ GPIOController)
	{

		return false;


	}

}
