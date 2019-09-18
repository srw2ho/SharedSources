#pragma once
#include "GPIOPin.h"
#include "GPIOEventPackageQueue.h"
using namespace Platform;
using namespace std;

namespace GPIODriver
{
	typedef std::vector<GPIOPin*> GPIOs;

    ref class GPIOInOut sealed

    {
	private:


#if !GPIOCLIENT_USING
		Windows::Devices::Gpio::GpioController ^ m_GPIOController;
		Windows::Devices::Pwm::PwmController ^ m_PWMController;			// PWM Software Controller
		Windows::Devices::Pwm::PwmController ^ m_PWMPCA9685Controller; // PWM Hardware Controller
		

#else

#endif
		GPIOs m_GPIOs; // 

//		Windows::ApplicationModel::AppService::AppServiceConnection^ m_connection;
		bool m_bInitialized;
		CRITICAL_SECTION m_CritLock;
		GPIODriver::GPIOEventPackageQueue* m_pGPIOEventPackageQueue;
    public:

		virtual ~GPIOInOut();
		bool InitGPIO();
		bool ResetGPIOPins(); // OutPut to Inititial Values
		bool DoProcessCommand(Platform::String^ command);
		Platform::String^ GetGPIState();
		Platform::String^ GetGPIStateByPinNr(int PinNr);



		Platform::String^ GetGPIClientSendState(); // Client State for Sending to GPIO Service

		void deleteAllGPIOPins();

		void UnLock();
		void Lock();

//		event Windows::Foundation::TypedEventHandler<Platform::Object^, Platform::String^>^ InputStateChanged;

//		event Windows::Foundation::TypedEventHandler<Platform::Object^, Platform::String^>^ OutputStateChanged;

//		event Windows::Foundation::TypedEventHandler<Platform::Object^, Platform::String^>^ InputHCSR04StateChanged;

		event Windows::Foundation::TypedEventHandler<Platform::Object^, int >^ CreateBME280External;

	internal:
		GPIOInOut(GPIODriver::GPIOEventPackageQueue* pGPIOEventPackageQueue);
		GPIOPin* getGPIOPinByPinNr(int Idx);
		bool addGPIOPin(GPIOPin*pin);
		bool delGPIOPin(GPIOPin*pin);
		bool GetGPIPinsByTyp(GPIOs & gPIOs, GPIODriver::GPIOTyp typ);

	private:
		bool DoParseCommand(const std::wstring  command);


		GPIOPin* InitGPIOPin(int PinNo, int TriggerPin, GPIODriver::GPIOTyp Typ, double SetValue,  double Pulsetime, double frequency);
		bool DoProcessGPIOCommand();
		/*
		void OnValueInputChanged(Windows::Devices::Gpio::GpioPin ^sender, Windows::Devices::Gpio::GpioPinValueChangedEventArgs ^args);
		void OnValueOutputChanged(Windows::Devices::Gpio::GpioPin ^sender, Windows::Devices::Gpio::GpioPinValueChangedEventArgs ^args);

		void OnValueHCSR04Changed(Windows::Devices::Gpio::GpioPin ^sender, Windows::Devices::Gpio::GpioPinValueChangedEventArgs ^args);


		bool setPinValue(GPIOPin* pin);
		*/

//		void OnValueInputChanged(Windows::Devices::Gpio::GpioPin ^ sender, Windows::Devices::Gpio::GpioPinValueChangedEventArgs ^ args);
		//Platform::String^ GetGPIState();
	};
}
