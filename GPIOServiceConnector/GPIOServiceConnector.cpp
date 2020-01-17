#include "pch.h"
#include "ServiceChunkReceiver.h"
#include "TimeConversion.h"

#include "GPIOServiceConnector.h"
#include "GPIODriver.h"



using namespace Platform;
using namespace Windows::Foundation;

using namespace Windows::Networking::Sockets;
using namespace Windows::UI::Core;
using namespace Windows::System::Threading;

using namespace StreamSocketComm;
using namespace concurrency;
using namespace GPIODriver;
using namespace GPIOServiceConnector;

const UINT64 nano100SecInSec = (UINT64)10000000 * 1;



GPIOConnector::GPIOConnector()
{
	m_pSocketListener = ref new StreamSocketComm::SocketListener();


	m_pGPIOEventPackageQueue = new GPIODriver::GPIOEventPackageQueue();

	m_GPIOClientInOut = ref new GPIODriver::GPIOInOut(m_pGPIOEventPackageQueue);

	//  m_pBME280DataQueue = new WeatherStationData::BME280DataQueue();



	m_bProcessingPackagesStarted = false;

	m_pPackageCancelTaskToken = nullptr;
	m_FailedEventRegister = m_pSocketListener->Failed += ref new Windows::Foundation::TypedEventHandler<Platform::Object ^, Platform::Exception ^>(this, &GPIOServiceConnector::GPIOConnector::OnFailed);

	m_OnClientConnected= m_pSocketListener->OnClientConnected += ref new Windows::Foundation::TypedEventHandler<Windows::Networking::Sockets::StreamSocket ^, int>(this, &GPIOServiceConnector::GPIOConnector::OnOnClientConnected);

	m_startStreamingEventRegister= m_pSocketListener->startStreaming += ref new Windows::Foundation::TypedEventHandler<Platform::Object ^, Windows::Networking::Sockets::StreamSocket ^>(this, &GPIOServiceConnector::GPIOConnector::OnstartStreaming);

	m_stopStreamingEventRegister= m_pSocketListener->stopStreaming += ref new Windows::Foundation::TypedEventHandler<Platform::Object ^, Platform::Exception^ >(this, &GPIOServiceConnector::GPIOConnector::OnstopStreaming);

	m_FailedConnectionCount = 0;
	m_inputconfigoptions = nullptr;
	m_outputconfigoptions = nullptr;



}

GPIOConnector::~GPIOConnector()
{
	// alle CallBacks zurücksetzen
	m_pSocketListener->Failed -= m_FailedEventRegister;
	m_pSocketListener->startStreaming -= m_startStreamingEventRegister;
	m_pSocketListener->stopStreaming -= m_stopStreamingEventRegister;
	m_pSocketListener->OnClientConnected -= m_OnClientConnected;

//	m_GPIOClientInOut->deleteAllGPIOPins();
	clearGPIOs();

	delete m_pSocketListener;
	delete m_pGPIOEventPackageQueue;
	m_pSocketListener = nullptr;
	m_pGPIOEventPackageQueue = nullptr;


}

void GPIOConnector::HostName::set(Platform::String ^ value) {
	m_HostName = value;
//	NotifyPropertyChanged("HostName");
//	NotifyPropertyChanged("VisibleKeyName");
}

void GPIOConnector::Port::set(int value) {
	m_port = value;
//	NotifyPropertyChanged("Port");
//	NotifyPropertyChanged("VisibleKeyName");
}




void GPIOConnector::FailedConnectionCount::set(unsigned int value) {
	m_FailedConnectionCount = value;

//	NotifyPropertyChanged("FailedConnectionCount");
}

Concurrency::task<void> GPIOConnector::doProcessPackages()
{
	auto token = m_pPackageCancelTaskToken->get_token();


	auto tsk = create_task([this, token]() -> void

	{
		bool dowhile = true;
		//	DWORD waitTime = INFINITE; // INFINITE ms Waiting-Time
		DWORD waitTime = 500; // 200 ms Waiting-Time

		// erst nach 2 sec mit dem Update loslegen, damit das System sich einschwingen kann
		UINT64 lastSendTime = ConversionHelpers::TimeConversion::getActualUnixTime() + 2 * nano100SecInSec;

	//	m_InletnextCycleFlushingTime = ConversionHelpers::TimeConversion::getActualUnixTime() + nano100SecInSec * 24 * 3600 * m_InletCycleFlushingTime;
		wchar_t szbuf[200];
	//	m_outputconfigoptions->Insert("State", dynamic_cast<PropertyValue^>(PropertyValue::CreateInt32(1)));
		bool doInit =true;
		while (dowhile) {
			try {

				GPIOEventPackage* pPacket = nullptr;
				UINT64 actualTime = ConversionHelpers::TimeConversion::getActualUnixTime();

				size_t startsize = m_pGPIOEventPackageQueue->waitForPacket(&pPacket, waitTime); //wait for incoming Packet, INFINITE waiting for incoming Package
				if (pPacket != nullptr)
				{
					if (m_outputconfigoptions != nullptr) {
						GPIOPin* pGPIOPin = (GPIOPin*)pPacket->getAdditional();
						if (pGPIOPin != nullptr)
						{
							switch (pGPIOPin->getGPIOTyp()) {
								case GPIOTyp::output:
								case GPIOTyp::input:
								{

					
									swprintf(szbuf, sizeof(szbuf), L"GPIO.%02d", pGPIOPin->getPinNumber());
									Platform::String^ keyValue = ref new Platform::String(szbuf);
																		
									if (m_outputconfigoptions->HasKey(keyValue)) {
										Platform::Object^ Value = m_outputconfigoptions->Lookup(keyValue);

										double actValue = safe_cast<IPropertyValue^>(Value)->GetDouble();
										double actpinValue = pGPIOPin->getPinValue();
										if (actValue != actpinValue) {
				
											m_outputconfigoptions->Insert(keyValue, dynamic_cast<PropertyValue^>(PropertyValue::CreateDouble(actpinValue)));
										//	if (startsize == 1) 
											{
												ChangeGPIOs(this, m_outputconfigoptions);
											}
				
										}

									}
					
									break;
								}
							}
						}
					}
					delete pPacket; // Package deleten
				}
				else {
				//	ChangeGPIOs(this, m_outputconfigoptions);
				}

				if (token.is_canceled()) {
					cancel_current_task();
				}

			}
			catch (task_canceled&)
			{
				dowhile = false;

			}
			catch (const exception&)
			{
				dowhile = false;

			}

		}

	}, token);

	return tsk;
}

void GPIOConnector::startProcessingPackages(Windows::Foundation::Collections::IPropertySet^ inputconfigoptions, Windows::Foundation::Collections::IPropertySet^ outputconfigoptions)
{
	//m_inputconfigoptions = inputconfigoptions;
	m_outputconfigoptions = outputconfigoptions;
	m_inputconfigoptions = inputconfigoptions;


	if (m_bProcessingPackagesStarted) return;
	bool doStart = false;
	if (inputconfigoptions->HasKey("HostName") && inputconfigoptions->HasKey("Port") ) {
		doStart = true;
	}
	if (!doStart) return;


	Platform::Object^ hostName = inputconfigoptions->Lookup("HostName");
	Platform::Object^ port = inputconfigoptions->Lookup("Port");
	if (hostName != nullptr) {
		this->m_HostName = safe_cast<IPropertyValue^>(hostName)->GetString();
	}
	if (port != nullptr) {
		this->m_port = safe_cast<IPropertyValue^>(port)->GetInt32();
	}


	m_bProcessingPackagesStarted = true;

	m_pGPIOEventPackageQueue->Flush();


	InitGPIOOutput(inputconfigoptions); // alle GPIO-Output erzeugen


	m_pSocketListener->StartClientAsync(m_HostName, m_port);



	if (m_pPackageCancelTaskToken != nullptr)
	{
		delete m_pPackageCancelTaskToken;

	}
	m_pPackageCancelTaskToken = new concurrency::cancellation_token_source();


	m_ProcessingPackagesTsk = create_task(doProcessPackages()).then([this](task<void> previous)
	{
		m_bProcessingPackagesStarted = false;
		try {
			previous.get();
		}
		catch (Exception^ exception)
		{

		}

	});




}

void GPIOConnector::cancelPackageAsync()
{
	if (m_pPackageCancelTaskToken != nullptr) {
		m_pPackageCancelTaskToken->cancel();
	}

	m_pGPIOEventPackageQueue->cancelwaitForPacket();




}

Windows::Foundation::IAsyncAction ^ GPIOConnector::startProcessingPackagesAsync(Windows::Foundation::Collections::IPropertySet^ inputconfigoptions, Windows::Foundation::Collections::IPropertySet^ outputconfigoptions) {


	return create_async([this, inputconfigoptions, outputconfigoptions]()
	{
		startProcessingPackages(inputconfigoptions, outputconfigoptions);

	});

}

Windows::Foundation::IAsyncAction ^ GPIOConnector::stopProcessingPackagesAsync() {

	return create_async([this]()
	{
		stopProcessingPackages();
	});
}


void GPIOConnector::stopProcessingPackages()
{
	if (!m_bProcessingPackagesStarted) return;
	try {

		if (m_inputconfigoptions != nullptr) {
			m_inputconfigoptions->MapChanged -= m_InputConfigOptionsMapChanged;
		}

		m_bProcessingPackagesStarted = false;
		m_pSocketListener->CancelConnections();// alle Connections schliessen

		cancelPackageAsync();


	//	Sleep(100);
		// Darf nicht in UI-Thread aufgerufen werden-> Blockiert UI-Thread-> gibt Exception
		m_ProcessingPackagesTsk.wait();

	//	m_outputconfigoptions->Insert("m_MovementActivated", dynamic_cast<PropertyValue^>(PropertyValue::CreateInt32(0)));
		m_outputconfigoptions->Insert("State", dynamic_cast<PropertyValue^>(PropertyValue::CreateInt32(0)));

		ChangeGPIOs(this, m_outputconfigoptions);

//		clearGPIOs();


	}
	catch (Exception^ exception)
	{
		bool b = true;

	}


}
void GPIOConnector::clearGPIOs() { // cancel all pending timeouts

	if (m_outputconfigoptions != nullptr) {
		m_outputconfigoptions->Clear();
	}

	m_Outputs.clear();
	m_GPIOClientInOut->deleteAllGPIOPins();

}


bool GPIOConnector::InitGPIOOutput(Windows::Foundation::Collections::IPropertySet^ inputconfigoptions) {

	clearGPIOs();

	if (inputconfigoptions == nullptr) return false;

	int PinNumber=-1;
	double InitValue;
	double SetValue = -1;
	double PinValue = -1;
	double PulseTime = -1;
	std::wstring s;
	std::wstring delim;



		auto options = inputconfigoptions->First();

		//	Platform::String^ folder, int fps, int height, int width, int64_t bit_rate, PropertySet^ ffmpegOutputOptions, Platform::String^ outputformat, double deletefilesOlderFilesinHours, double RecordingInHours


		std::wstring  GPIOName;
		std::wstring  GPIOTyp;

		while (options->HasCurrent)
		{
			Platform::String^ key = options->Current->Key;
			Platform::Object^ value = options->Current->Value;
			std::wstring working_key = key->Data();
			std::wstring insertKey;

			GPIODriver::remove(working_key, (L" ")[0]);
			insertKey = working_key;


			std::wstring key_Value;
			std::wstring keyPin;
			std::wstring Key;

			bool doProcess = false;
			std::vector<std::wstring> _Keyarray = GPIODriver::splitintoArray(working_key, L".");
			if (_Keyarray.size() > 0) // Key vom Typ GPIO.10
			{
		//		GPIODriver::remove(_Keyarray[0], (L" ")[0]);
				if (_Keyarray[0] == L"GPIO")
				{

						Platform::String^ keyValue = safe_cast<IPropertyValue^>(value)->GetString();

						std::wstring  working_keyValue = keyValue->Data();
						GPIODriver::remove(working_keyValue, (L" ")[0]);

						std::vector<std::wstring> Keyarray = GPIODriver::splitintoArray(working_keyValue, L";");
						//         string keyValue = string.Format("PinName={0}; Typ={1}; PinNumber:{2}; InitValue={3}; SetValue={4}", m_GPIOName, m_GPIOTyp.ToString(), m_PinNumber, m_InitValue,m_SetValue-SetValue); ;
						std::vector<std::wstring> valueArr;
		


						for (size_t i = 0; i < Keyarray.size(); i++) {
							std::wstring Keys = Keyarray.at(i);
						//	GPIODriver::remove(Keys, (L" ")[0]);
							valueArr = GPIODriver::splitintoArray(Keys, L"=");
							if (valueArr.size() > 0) {

								key_Value = valueArr.at(1);
								Key = valueArr.at(0);

								if (Key == L"PinName") {

									GPIOName = key_Value;
								}
								else if (Key == L"Typ") {
									GPIOTyp = key_Value;
								}
								else if (Key == L"SetValue") {
									SetValue = _wtof(key_Value.c_str());
								}
								else if (Key == L"PinNumber") {
									PinNumber = (int)_wtof(key_Value.c_str());
								}
								else if (Key == L"InitValue") {
									InitValue = _wtof(key_Value.c_str());
								}
								else if (Key == L"PinValue") {
									PinValue = _wtof(key_Value.c_str());
								}
								else if (Key == L"PulseTime") {
									PulseTime = _wtof(key_Value.c_str());
								}
								

								if ((SetValue != -1) && (PinNumber != -1) && (PinValue!=-1) && (GPIOTyp.size() > 0)) {
									GPIODriver::GPIOPin* pPin = nullptr;

									if (GPIOTyp == L"input") {

										pPin = new GPIODriver::GPIOInputPin(m_pGPIOEventPackageQueue, PinNumber, (InitValue > 0) ? 0 : 1);
										pPin->setActivateOutputProcessing(true);
									}
									else if (GPIOTyp == L"output") {
										pPin = new GPIODriver::GPIOOutputPin(m_pGPIOEventPackageQueue, PinNumber, (InitValue > 0) ? 0 : 1);
										// beim Initialisieren keine Pulse-Time beachten
										//if (PulseTime >= 0) {
										//	pPin->setPulseTimeinms(PulseTime);
										//}
										pPin->setActivateOutputProcessing(true);
									}
									if (pPin != nullptr) {
										m_GPIOClientInOut->addGPIOPin(pPin);
										Platform::String^ key = ref new Platform::String(insertKey.c_str());
										pPin->setSetValue(PinValue);
										this->m_outputconfigoptions->Insert(key, dynamic_cast<PropertyValue^>(PropertyValue::CreateDouble(PinValue)));

									}
									PulseTime = -1;
									SetValue = -1;
									PinNumber = -1;
									PinValue = -1;
									GPIOTyp.clear();
								}
							}




						}
					}

			}
			//   

			options->MoveNext();
		}


		m_GPIOClientInOut->GetGPIPinsByTyp(m_Outputs, GPIODriver::GPIOTyp::output);
		if (m_Outputs.size() > 0) {
			m_InputConfigOptionsMapChanged = inputconfigoptions->MapChanged += ref new Windows::Foundation::Collections::MapChangedEventHandler<Platform::String ^, Platform::Object ^>(this, &GPIOServiceConnector::GPIOConnector::OnMapChanged);

		}

		m_outputconfigoptions->Insert("State", dynamic_cast<PropertyValue^>(PropertyValue::CreateInt32(-1)));


		//this->ChangeGPIOs(this, m_outputconfigoptions);



return true;
}

void GPIOServiceConnector::GPIOConnector::OnFailed(Platform::Object ^sender, Platform::Exception ^args)
{// Trying to connect failed
//	throw ref new Platform::NotImplementedException();
	Platform::String^ err = ref new String();
	err = args->Message;
	m_FailedConnectionCount = m_FailedConnectionCount + 1;
	this->Failed(this, err);
}


void GPIOServiceConnector::GPIOConnector::OnOnClientConnected(Windows::Networking::Sockets::StreamSocket ^sender, int args)
{// Client connection created --> Add Connection to Listener
	//throw ref new Platform::NotImplementedException();

	StreamSocket^ socket = sender;

	StreamSocketComm::SocketChunkReceiver* pServiceListener = new ServiceChunkReceiver(socket, this->m_GPIOClientInOut);

	SocketChunkReceiverWinRT ^ pBME280ChunkReceiverWinRT = ref new SocketChunkReceiverWinRT(pServiceListener); // wrapper for SocketChunkReceiver and its derived
	m_pSocketListener->AddChunkReceiver(pBME280ChunkReceiverWinRT);
	pBME280ChunkReceiverWinRT->geSocketChunkReceiver()->StartService(); // send "BME280Server.Start" - Copmmand to Client Station-> Start with Communication
//		this->m_FailedConnectionCount = 0; // wieder 0 setzen, damit Fehler wieder neu erfasst werden kann.




}


void GPIOServiceConnector::GPIOConnector::OnstartStreaming(Platform::Object ^sender, Windows::Networking::Sockets::StreamSocket ^args)
{
	m_FailedConnectionCount = 0;
	this->startStreaming(this, args);
	//throw ref new Platform::NotImplementedException();
}


void GPIOServiceConnector::GPIOConnector::OnstopStreaming(Platform::Object ^sender, Platform::Exception ^exception)
{
	Platform::String^ err = ref new String();
	if (exception != nullptr) {
		err = exception->Message;
		m_FailedConnectionCount = m_FailedConnectionCount + 1;
	}
	else err = "";

	this->stopStreaming(this, err);
	//Connection  error by remote or stopped by user (CancelConnections)
	//throw ref new Platform::NotImplementedException();
}


void GPIOServiceConnector::GPIOConnector::OnMapChanged(Windows::Foundation::Collections::IObservableMap<Platform::String ^, Platform::Object ^> ^sender, Windows::Foundation::Collections::IMapChangedEventArgs<Platform::String ^> ^event)
{
	auto propertyset = sender;

	//	Platform::String^ folder, int fps, int height, int width, int64_t bit_rate, PropertySet^ ffmpegOutputOptions, Platform::String^ outputformat, double deletefilesOlderFilesinHours, double RecordingInHours

	if (propertyset->HasKey("UpdateState")) {
		Platform::Object^ Value = propertyset->Lookup("UpdateState");
		int state = safe_cast<IPropertyValue^>(Value)->GetInt32();
		if (state != 1) { // es folgend noch weitere
			return;
		}
	}
	int Idx = 0;
	//	double SetValue;
	GPIODriver::GPIOPin* pGPIOPin;
	std::wstring _state;

	//GPIOs  gPIOs;
	//m_GPIOClientInOut->GetGPIPinsByTyp(gPIOs, GPIODriver::GPIOTyp::output);
	for (size_t i = 0; i < m_Outputs.size(); i++) {
		pGPIOPin = m_Outputs.at(i);



		wchar_t szbuf[200];
		swprintf(szbuf, sizeof(szbuf), L"GPIO.%02d", pGPIOPin->getPinNumber());
		Platform::String^ keyValue = ref new Platform::String(szbuf);

		if (propertyset->HasKey(keyValue)) {

			Platform::Object^ Value = propertyset->Lookup(keyValue);
			Platform::String^ actValue = safe_cast<IPropertyValue^>(Value)->GetString();
			std::wstring  working_keyValue = actValue->Data();
			GPIODriver::remove(working_keyValue, (L" ")[0]);

			std::vector<std::wstring> Keyarray = GPIODriver::splitintoArray(working_keyValue, L";");
			//         string keyValue = string.Format("PinName={0}; Typ={1}; PinNumber:{2}; InitValue={3}; SetValue={4}", m_GPIOName, m_GPIOTyp.ToString(), m_PinNumber, m_InitValue,m_SetValue-SetValue); ;
			std::vector<std::wstring> valueArr;
			std::wstring  key_Value;
			std::wstring  Key;
			double SetValue;
			double PulseTime = 0;
			bool IsFlankActive = true;
			for (size_t i = 0; i < Keyarray.size(); i++) {
				std::wstring Keys = Keyarray.at(i);
				//	GPIODriver::remove(Keys, (L" ")[0]);
				valueArr = GPIODriver::splitintoArray(Keys, L"=");
				if (valueArr.size() > 0) {

					key_Value = valueArr.at(1);
					Key = valueArr.at(0);
					if (Key == L"PulseTime") {
						PulseTime = _wtof(key_Value.c_str());
					}
					if (Key == L"IsFlankActive") {
						IsFlankActive = (_wtof(key_Value.c_str()) > 0);
	//					wistringstream(key_Value.c_str()) >> std::boolalpha >> IsFlankActive;
					}
					if (Key == L"SetValue") {
						SetValue = _wtof(key_Value.c_str());
						if (SetValue != pGPIOPin->getSetValue()) {
							pGPIOPin->setPulseTimeinms(PulseTime);
							pGPIOPin->setSetValue(SetValue);
							if (IsFlankActive) {
								Platform::String^ state = pGPIOPin->GetGPIOPinClientSendCmd();
								if (state->Length() > 0) {
									_state.append(state->Data());
								}
							}

						}

					}

				}

			}
		}

	}


	if (_state.size() > 0) {
		Platform::String^ state = ref new Platform::String(_state.c_str());

		Windows::Storage::Streams::IBuffer^ buf = SocketHelpers::createPayloadBufferfromSendData(state);
		m_pSocketListener->SendDataToClients(buf);
	}


}
