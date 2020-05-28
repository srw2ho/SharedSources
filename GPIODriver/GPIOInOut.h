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

		bool m_bInitialized;
		CRITICAL_SECTION m_CritLock;
		GPIODriver::GPIOEventPackageQueue* m_pGPIOEventPackageQueue;
		bool m_UseMpack;
    public:

		virtual ~GPIOInOut();
		void setUseMpack(bool value);
		bool getUseMpack() { return m_UseMpack; };

		bool InitGPIO();
		bool ResetGPIOPins(); // OutPut to Inititial Values


		Windows::Storage::Streams::IBuffer^ GetGPIStateBuf();

		Windows::Storage::Streams::IBuffer^ GetGPIClientSendStateBuf();


		void deleteAllGPIOPins();

		void UnLock();
		void Lock();

		event Windows::Foundation::TypedEventHandler<Platform::Object^, int >^ CreateBME280External;

	internal:
		GPIOInOut(GPIODriver::GPIOEventPackageQueue* pGPIOEventPackageQueue);
		GPIOPin* getGPIOPinByPinNr(int Idx);
		GPIOPin* getGPIOPinByPinNrandType(GPIODriver::GPIOTyp type, int Idx);
		bool addGPIOPin(GPIOPin*pin);
		bool delGPIOPin(GPIOPin*pin);
		bool GetGPIPinsByTyp(GPIOs & gPIOs, GPIODriver::GPIOTyp typ);



		bool parse_mpackObjects(std::vector<byte>& arr);
		bool DoProcessCommand(Platform::String^ command);

	private:
		bool DoParseCommand(const std::wstring  command);
		bool parse_mpegandgetObject(mpack_reader_t* reader);

		GPIOPin* InitGPIOPin(int PinNo, int TriggerPin, GPIODriver::GPIOTyp Typ, double SetValue,  double Pulsetime, double frequency);
		bool DoProcessGPIOCommand();
		Platform::String^ GetGPIClientSendState(); // Client State for Sending to GPIO Service
		Platform::String^ GetGPIState();
		Platform::String^ GetGPIStateByPinNr(int PinNr);


		bool GetGPIMsgPackState(std::vector<char>& retdata);
		std::vector<char> GetGPIMsgPackStateByPinNr(int PinNr);
		bool GetGPIClientMsPackSendState(std::vector<char>& retdata);
	};
}
