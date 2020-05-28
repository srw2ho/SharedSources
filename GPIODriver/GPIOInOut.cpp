#include "pch.h"
#include "GPIODriver.h"
#include "GPIOInOut.h"

using namespace GPIODriver;
using namespace Platform;
using namespace Windows::ApplicationModel::AppService;



using namespace Windows::Devices::Pwm;
using namespace Windows::Devices::Gpio;
using namespace Windows::Foundation;
using namespace concurrency;
using namespace Windows::System::Threading;
using namespace Windows::Foundation::Collections;

#if !GPIOCLIENT_USING
using namespace Microsoft::IoT::Lightning::Providers;
#else

#endif


// GPO.1 = 1;	: for setting GPIO Pin 1 set to 1
// GOO.5 = 0, Pulsetime=1000;		// Pulse-Time = 1000 ms
// GPI.1 = ?;
// GPI.5 = ?;	: for reading GPI Pin 5


// PWM.27 = -1;		: for Stopping PWM Out
// PWM.27 = 0.1;	: for >=0 Duty and PWM-Start

#define PWM_Frequency		50

const UINT64 nano100SecInSec = (UINT64)10000000 * 1;

namespace GPIODriver
{
	GPIOInOut::GPIOInOut(GPIODriver::GPIOEventPackageQueue* pGPIOEventPackageQueue)
	{
		m_pGPIOEventPackageQueue = pGPIOEventPackageQueue;
		InitializeCriticalSection(&m_CritLock);
#if !GPIOCLIENT_USING
		m_PWMController = nullptr;
		m_PWMPCA9685Controller = nullptr;
		m_GPIOController = nullptr;
#else

#endif


		m_bInitialized = false;
		m_UseMpack = false;

	}

	GPIOInOut::~GPIOInOut() {

		DeleteCriticalSection(&m_CritLock);
	}

	void GPIOInOut::setUseMpack(bool value) {
		m_UseMpack = value;
		m_pGPIOEventPackageQueue->setUseMpack(value);
	};

	void GPIOInOut::UnLock() {

		LeaveCriticalSection(&m_CritLock);
	}

	void GPIOInOut::Lock() {
		EnterCriticalSection(&m_CritLock);
	}


	bool GPIOInOut::DoParseCommand(const std::wstring  command) {

		GPIODriver::GPIOTyp Typ = GPIODriver::GPIOTyp::undef;
		int Pin = -1;
		//	const std::wstring s = command->Data();
		int value = -1;
		double pulseValue = -1;
		int trigPin = -1;

		double SetValue = value;

		double pressure = -1;
		double temperature = -1;
		double humidity = -1;
		double frequency = -1;
		UINT64 timeInNanos = -1;

		std::wstring singlecmd = command;

		std::vector<std::wstring> commands = splitintoArray(command, L",");
		if (commands.size() >= 0)
		{
			singlecmd = commands[0];

			for (size_t k=1; k < commands.size(); k++) // im Command sind weitere enthalen
			{
				std::vector<std::wstring> furtherCommand = splitintoArray(commands[k], L"=");
				if (furtherCommand.size() > 0) {
					if (wcscmp(furtherCommand[0].c_str(), L"Pulsetime") == 0) // for GPO.5, Pulsetime = 10;
					{
						pulseValue = _wtof(furtherCommand[1].c_str());
					}
					else if (wcscmp(furtherCommand[0].c_str(), L"Frequ") == 0) // for GPO.5, Pulsetime = 10;
					{
						frequency = _wtof(furtherCommand[1].c_str());
					}
					else
					if (wcscmp(furtherCommand[0].c_str(), L"TrigPin") == 0) // for HC_SR04.5, TrigPin = 20;
					{
							trigPin = _wtoi(furtherCommand[1].c_str());
					}
					else
					if (wcscmp(furtherCommand[0].c_str(), L"Temp") == 0) // for HC_SR04.5, TrigPin = 20;
					{
						temperature = _wtof(furtherCommand[1].c_str());
					}
					else 
					if (wcscmp(furtherCommand[0].c_str(), L"Press") == 0) // for HC_SR04.5, TrigPin = 20;
					{
						pressure = _wtof(furtherCommand[1].c_str());
					}
					else
					if (wcscmp(furtherCommand[0].c_str(), L"Humid") == 0) // for HC_SR04.5, TrigPin = 20;
					{
						humidity = _wtof(furtherCommand[1].c_str());
					}
					else
					if (wcscmp(furtherCommand[0].c_str(), L"TimeNs") == 0) // actSystem Time in nanoSec
					{
						UINT64 dmy = getActualTimeinNanos();
						timeInNanos = ConvertStringToUINT64(furtherCommand[1].c_str());
					}
				}

			}

		}

		std::vector<std::wstring> arr = splitintoArray(singlecmd, L"=");
		if (arr.size() > 1) {

			double value = _wtof(arr[1].c_str());

			std::vector<std::wstring> _PinNo = splitintoArray(arr[0].c_str(), L".");
			if (_PinNo.size() > 1)
			{
				int parsePin = _wtoi(_PinNo[1].c_str());
				Pin = parsePin;

				if (wcscmp(_PinNo[0].c_str(), L"GPO") == 0) { // OutPut
					Typ = GPIODriver::GPIOTyp::output;
					SetValue = value;

				}
				else if (wcscmp(_PinNo[0].c_str(), L"GPI") == 0) { // Input
					Typ = GPIODriver::GPIOTyp::input;
					SetValue = value;

				}
				else if (wcscmp(_PinNo[0].c_str(), L"GPIShutDown") == 0) { // Input-GPIShutDown
					Typ = GPIODriver::GPIOTyp::inputShutdown;
					SetValue = value;

				}
				
				else if (wcscmp(_PinNo[0].c_str(), L"PWM") == 0) { // Input
					Typ = GPIODriver::GPIOTyp::PWM;
					SetValue = value;

				}
				else if (wcscmp(_PinNo[0].c_str(), L"PWM9685") == 0) { // Input
					Typ = GPIODriver::GPIOTyp::PWM9685;
					SetValue = value;

				}
				else if (wcscmp(_PinNo[0].c_str(), L"HC_SR04") == 0) { // Input
					Typ = GPIODriver::GPIOTyp::HC_SR04;
					SetValue = value;

				}
				else if (wcscmp(_PinNo[0].c_str(), L"BME280") == 0) { // Input
					Typ = GPIODriver::GPIOTyp::BME280;
					SetValue = value;
				}

				GPIOPin* pGPIOPin = getGPIOPinByPinNr(parsePin);
				if ((pGPIOPin != nullptr) && (pGPIOPin->getGPIOTyp() != Typ))
				{
					this->delGPIOPin(pGPIOPin);
					pGPIOPin = nullptr;
				}

				if (pGPIOPin == nullptr)
				{
					pGPIOPin = InitGPIOPin(parsePin, trigPin, Typ, SetValue, pulseValue, frequency);
				}
				if (pGPIOPin != nullptr)
				{
					if (pGPIOPin->getGPIOTyp() == GPIODriver::GPIOTyp::output) // output
					{
#if !GPIOCLIENT_USING
						pGPIOPin->setSetValue(SetValue);
						pGPIOPin->setPulseTimeinms(pulseValue);
#else
						pGPIOPin->setPinValue(SetValue);
						pGPIOPin->setTimeinNanos(timeInNanos);

#endif

						pGPIOPin->setActivateProcessing(true);
					}
					else if ((pGPIOPin->getGPIOTyp() == GPIODriver::GPIOTyp::PWM) || (pGPIOPin->getGPIOTyp() == GPIODriver::GPIOTyp::PWM9685) ) // PWN-output
					{
						GPIOPWMOutputPin*pPin = (GPIOPWMOutputPin*)pGPIOPin;
#if !GPIOCLIENT_USING
						pPin->setSetValue(SetValue);
						if ( (frequency != -1) && (frequency != pPin->getFrequency()) ){
							if (pGPIOPin->getGPIOTyp() == GPIODriver::GPIOTyp::PWM)
							{
								pPin->UpdatesetFrequency(this->m_PWMController, frequency);
							}
							else
							{
								pPin->UpdatesetFrequency(this->m_PWMPCA9685Controller, frequency);
							}
						}

#else
						pPin->setPinValue(SetValue);
						pPin->setTimeinNanos(timeInNanos);

#endif
						pPin->setFrequency(frequency);
						pPin->setActivateProcessing(true);
					}
					else if (pGPIOPin->getGPIOTyp() == GPIODriver::GPIOTyp::input) // input
					{

#if !GPIOCLIENT_USING

#else
						pGPIOPin->setPinValue(SetValue); // Übernahme der Daten
						pGPIOPin->setActivateProcessing(true);
						pGPIOPin->setTimeinNanos(timeInNanos);

#endif
					}
					else if (pGPIOPin->getGPIOTyp() == GPIODriver::GPIOTyp::inputShutdown) // input
					{

#if !GPIOCLIENT_USING

#else
						pGPIOPin->setPinValue(SetValue); // Übernahme der Daten
						pGPIOPin->setActivateProcessing(true);
						pGPIOPin->setTimeinNanos(timeInNanos);

#endif
					}
					else if (pGPIOPin->getGPIOTyp() == GPIODriver::GPIOTyp::HC_SR04) // HC_SR04 special Behavior
					{
#if !GPIOCLIENT_USING

#else
						pGPIOPin->setPinValue(SetValue); // Übernahme der Daten

#endif
						pGPIOPin->setActivateProcessing(true);
						pGPIOPin->setTimeinNanos(timeInNanos);
					}
					else if (pGPIOPin->getGPIOTyp() == GPIODriver::GPIOTyp::BME280) // HC_SR04 special Behavior
					{
#if !GPIOCLIENT_USING

#else
						BME280Sensor*pSensor =(BME280Sensor*) pGPIOPin;
						pSensor->setPressure(pressure);
						pSensor->setTemperature (temperature);
						pSensor->setHumidity(humidity);
						pGPIOPin->setPinValue(SetValue); // Übernahme der Daten
						pGPIOPin->setTimeinNanos(timeInNanos);

#endif
						pGPIOPin->setActivateProcessing(true);
					}
				}

			}

		}



		return false;

	}
	bool GPIOInOut::DoProcessCommand(Platform::String^ command) {


		double pulseValue = 0;
		int trigPin = 0;
		wchar_t removechar = ' ';

		std::wstring  workcommand = command->Data();;
		remove(workcommand, (wchar_t)removechar); // remove blanks form string

		const std::wstring s = workcommand;

		Lock();

		std::vector<std::wstring> arr = splitintoArray(s, L";"); // commata serperated string
		std::wstring cmd;
		for (size_t i = 0; i < arr.size(); i++) {

			bool bDoParse = DoParseCommand(arr[i]);
		}

		UnLock();

		DoProcessGPIOCommand();

		return true;

	}



	Platform::String^ GPIOInOut::GetGPIStateByPinNr(int PinNr) {

		Platform::String^ ret = L"";
		Lock();
		GPIOPin*pPin = this->getGPIOPinByPinNr(PinNr);
		if (pPin != nullptr) {
			ret= pPin->GetGPIOPinCmd();
		}
		UnLock();
		return ret;

	}


	bool GPIOInOut::GetGPIMsgPackState(std::vector<char>& retdata) {
	//	std::vector<char> retdata;

		Lock();
		for (size_t i = 0; i < m_GPIOs.size(); i++) {
			auto pin = m_GPIOs.at(i);
			std::vector<char> data(200);
			size_t len = 0;
			pin->doPack(&data[0], &len);
			if (len > 0) {
				data.resize(len);
				retdata.insert(retdata.end(), data.begin(), data.end()); // note: we use different vectors here
			}
		}
		UnLock();
	

		return (retdata.size()>0);
	}


	std::vector<char> GPIOInOut::GetGPIMsgPackStateByPinNr(int PinNr) {
		std::vector<char> retdata(200);
	
		Lock();
		GPIOPin* pPin = this->getGPIOPinByPinNr(PinNr);
		if (pPin != nullptr) {
			size_t len = 0;
			pPin->doPack(&retdata[0], &len);
			if (len > 0) {
				retdata.resize(len);
			}
		}
		UnLock();
		return retdata;
	}
	

	bool GPIOInOut::GetGPIClientMsPackSendState(std::vector<char>& retdata) {
	
		Lock();
	
		for (size_t i = 0; i < m_GPIOs.size(); i++) {
			auto pin = m_GPIOs.at(i);
			if (pin->getActivateOutputProcessing())
			{
				std::vector<char> data(200);
				size_t len = 0;
				pin->doPack(&data[0], &len);

				if (len > 0) {
					data.resize(len);
					retdata.insert(retdata.end(), data.begin(), data.end()); // note: we use different vectors here
				}
			}

		}
		UnLock();
		return (retdata.size() > 0) ;
	}


	Windows::Storage::Streams::IBuffer^ GPIOInOut::GetGPIStateBuf() {
		Windows::Storage::Streams::IBuffer^ buf=nullptr;
		Lock();
		if (m_UseMpack) {
			std::vector<char> retdata;
			GetGPIMsgPackState(retdata);
			if (retdata.size() > 0) {
				buf = createPayloadBufferfromMpackData(retdata);
			}
		}
		else {
			Platform::String^ state = GetGPIState();
			if (state->Length() > 0) {

				buf = createPayloadBufferfromSendData(state);
			}
		}

		UnLock();
		return buf;
	}

	Windows::Storage::Streams::IBuffer^ GPIOInOut::GetGPIClientSendStateBuf() {
		Windows::Storage::Streams::IBuffer^ buf=nullptr;
		Lock();
		if (m_UseMpack) {
			std::vector<char> retdata;
			GetGPIClientMsPackSendState(retdata);
			if (retdata.size() > 0){
				buf = createPayloadBufferfromMpackData(retdata);
			}

		}
		else {
			Platform::String^ state = GetGPIClientSendState();

			if (state->Length() > 0) {
				buf = createPayloadBufferfromSendData(state);
			}

		}

		UnLock();
		return buf;
	}


	Platform::String^ GPIOInOut::GetGPIClientSendState() {
		std::wstring _state;
		_state.clear();
		Lock();
		for (size_t i = 0; i < m_GPIOs.size(); i++) {
			auto pin = m_GPIOs.at(i);
			if (pin->getActivateOutputProcessing())
			{
				Platform::String^ state = pin->GetGPIOPinClientSendCmd();
				if (state->Length() > 0) {
					_state.append(state->Data());
				}
			}

		}
		Platform::String^ state = ref new Platform::String(_state.c_str());
		UnLock();
		return state;
	}

	bool GPIOInOut::GetGPIPinsByTyp(GPIOs & gPIOs, GPIODriver::GPIOTyp typ)
	{
		Lock();
		for (size_t i = 0; i < m_GPIOs.size(); i++) {
			auto pin = m_GPIOs.at(i);
			if (pin->getGPIOTyp() == typ)
			{
				gPIOs.push_back(pin);
			}
			
		}
		UnLock();
		return (gPIOs.size() > 0);
	}

	Platform::String^ GPIOInOut::GetGPIState() {
		std::wstring _state;
		_state.clear();
		Lock();
		for (size_t i = 0; i < m_GPIOs.size(); i++) {
			auto pin = m_GPIOs.at(i);
			Platform::String^ state  = pin->GetGPIOPinCmd();
			if (state->Length() > 0)
			{
				_state.append(state->Data());
			}
		}
		Platform::String^ state = ref new Platform::String(_state.c_str());
		UnLock();
		return state;
	}

	GPIOPin* GPIOInOut::InitGPIOPin(int PinNo, int TriggerPin, GPIODriver::GPIOTyp Typ, double SetValue, double Pulsetime, double frequency) {
		GPIOPin* pRetGPIOPin = nullptr;

#if !GPIOCLIENT_USING
		Lock();
		if (Typ == GPIODriver::GPIOTyp::input) {
			GPIOInputPin* pGPIOPin = nullptr;

			pGPIOPin = new GPIOInputPin(this->m_pGPIOEventPackageQueue, PinNo, SetValue);
			bool InitOK = pGPIOPin->Init(m_GPIOController);

			if (!InitOK)
			{
				delete pGPIOPin;
				pRetGPIOPin = nullptr;
			}
			else pRetGPIOPin = pGPIOPin;

		}
		else if (Typ == GPIODriver::GPIOTyp::inputShutdown) {
			GPIOInputPin* pGPIOPin = nullptr;

			pGPIOPin = new GPIOInputPinShutDown(this->m_pGPIOEventPackageQueue, PinNo, SetValue);
			bool InitOK = pGPIOPin->Init(m_GPIOController);

			if (!InitOK)
			{
				delete pGPIOPin;
				pRetGPIOPin = nullptr;
			}
			else pRetGPIOPin = pGPIOPin;

		}
		else if (Typ == GPIODriver::GPIOTyp::output) {

			GPIOOutputPin* pGPIOPin = nullptr;

			pGPIOPin = new GPIOOutputPin(this->m_pGPIOEventPackageQueue, PinNo, SetValue);
			pGPIOPin->setPulseTimeinms(Pulsetime);

			bool InitOK = pGPIOPin->Init(m_GPIOController);

			if (!InitOK)
			{
				delete pGPIOPin;
				pRetGPIOPin = nullptr;
			}
			else pRetGPIOPin = pGPIOPin;

		}
		else if ( (Typ == GPIODriver::GPIOTyp::PWM) || (Typ == GPIODriver::GPIOTyp::PWM9685 )) {

			GPIOPWMOutputPin* pGPIOPin = nullptr;
			bool InitOK;
			
			if (Typ == GPIODriver::GPIOTyp::PWM)
			{
				pGPIOPin = new GPIOPWMOutputPin(this->m_pGPIOEventPackageQueue, PinNo, SetValue);
				InitOK = pGPIOPin->Init(this->m_PWMController, frequency);
			}
			else
			{
				pGPIOPin = new GPIOPWMPWM9685OutputPin(this->m_pGPIOEventPackageQueue, PinNo, SetValue);
				InitOK = pGPIOPin->Init(this->m_PWMPCA9685Controller, frequency);
			}


			if (!InitOK)
			{
				delete pGPIOPin;
				pRetGPIOPin = nullptr;
			}
			else {
				pRetGPIOPin = pGPIOPin;

			}

		}
		else if (Typ == GPIODriver::GPIOTyp::HC_SR04) {

			GPIOHCSR04* pGPIOPin = nullptr;

			pGPIOPin = new GPIOHCSR04(this->m_pGPIOEventPackageQueue, PinNo, TriggerPin, SetValue);
			bool InitOK = pGPIOPin->Init(this->m_GPIOController);

			if (!InitOK)
			{
				delete pGPIOPin;
				pRetGPIOPin = nullptr;
			}
			else pRetGPIOPin = pGPIOPin;

		}
		else if (Typ == GPIODriver::GPIOTyp::BME280) {

			CreateBME280External(this,(int) PinNo);
			pRetGPIOPin = nullptr;
	//		pRetGPIOPin = getGPIOPinByPinNr(PinNo);

		}

		if (pRetGPIOPin != nullptr) {
			addGPIOPin(pRetGPIOPin);
		}
		UnLock();
		return pRetGPIOPin;
#else
		return nullptr;
#endif

	}





	bool GPIOInOut::InitGPIO() {

#if !GPIOCLIENT_USING
		try {

			if (m_bInitialized) return true;

			//	if (LightningProvider::IsLightningEnabled)
			if (Microsoft::IoT::Lightning::Providers::LightningProvider::IsLightningEnabled)
			{
				Windows::Devices::LowLevelDevicesController::DefaultProvider = LightningProvider::GetAggregateProvider();

			}
			auto pwmControllers = PwmController::GetControllersAsync(LightningPwmProvider::GetPwmProvider());

			//	auto controller = pwmControllers[0];

			create_task(pwmControllers).then([this](Windows::Foundation::Collections::IVectorView<PwmController^>^ pwmControllers) {
				if (pwmControllers->Size > 0) {
					if (pwmControllers->Size > 1) {
						auto pwmController = pwmControllers->GetAt(1); // the on-device controller:: LightningSoftwarePwmControllerProvider
						m_PWMController = pwmController;

					}
					if (pwmControllers->Size > 0) {
						auto pwmPCA9685Controller = pwmControllers->GetAt(0); // the on-device controller:: LightningSoftwarePwmControllerProvider
						m_PWMPCA9685Controller = pwmPCA9685Controller;
					}
				}

			});

			m_GPIOController = GpioController::GetDefault();
			m_bInitialized = true;
		}
		catch (Exception ^ ex) {

			return false;
		}

		return (m_GPIOController != nullptr);
#else
		return false;
#endif


	}


	bool GPIOInOut::addGPIOPin(GPIOPin*pin) {
		Lock();
		m_GPIOs.push_back(pin);
		UnLock();
		return true;
	}

	bool GPIOInOut::delGPIOPin(GPIOPin*pintoDelete) {
		Lock();
		for (size_t i = 0; i < m_GPIOs.size(); i++) {
			auto pin = m_GPIOs.at(i);
			if (pin == pintoDelete) {
				pin->DiableValueChanged(); // vor dem Löschen das Interrupt-Handling stoppen!!
				m_GPIOs.erase(m_GPIOs.begin() + i);
				delete pin;
				UnLock();
				return true;
			}

		}
		UnLock();
		return false;
	}

	GPIOPin* GPIOInOut::getGPIOPinByPinNr(int Idx) {
		GPIOPin*pRet = nullptr;
		Lock();
		for (size_t i = 0; i < m_GPIOs.size(); i++) {
			auto pin = m_GPIOs.at(i);
			if (pin->getPinNumber() == Idx) {
				UnLock();
				return pin;
			}
		}
		UnLock();
		return nullptr;
	}
	GPIOPin* GPIOInOut::getGPIOPinByPinNrandType(GPIODriver::GPIOTyp type, int Idx) {
		GPIOPin* pRet = nullptr;
		Lock();
		for (size_t i = 0; i < m_GPIOs.size(); i++) {
			auto pin = m_GPIOs.at(i);
			if ((pin->getPinNumber() == Idx) && (pin->getGPIOTyp() == type)) {
				UnLock();
				return pin;
			}
		}
		UnLock();
		return nullptr;
	}




	void GPIOInOut::deleteAllGPIOPins() {
		Lock();
		while (m_GPIOs.size() > 0)
		{
			auto pin = m_GPIOs.front();
			pin->DiableValueChanged(); // vor dem Löschen das Interrupt-Handling stoppen!!
			m_GPIOs.erase(m_GPIOs.begin());
			delete pin;

		}
		UnLock();
	}



	bool GPIOInOut::DoProcessGPIOCommand() {

		bool bret = false;
		Lock();
		for (size_t i = 0; i < m_GPIOs.size(); i++) {
			auto pin = m_GPIOs.at(i);
			if (pin->getActivateProcessing())
			{

#if !GPIOCLIENT_USING
		
				pin->doProcessing();		// Process Hardware - Pin Information
				pin->setActivateProcessing(false);

#else
				pin->doClientProcessing(); // Process Client Information
				
#endif

				bret = true;
			}
		}
		UnLock();

		return (bret);

	}

	
	bool GPIOInOut::ResetGPIOPins() {

		Lock();
		for (size_t i = 0; i < m_GPIOs.size(); i++) {
			auto pin = m_GPIOs.at(i);
			pin->setOutputToInitialValue();
		}
		UnLock();
		return (m_GPIOs.size ()> 0);
	}
	
}

