#pragma once
#include <wrl.h>
#include <robuffer.h>

//#include <SerDevice.h>
#include <SerialChunkreceiver.h>

namespace SerialCommunication
{
	ref class SerialChunkReceiverWinRT sealed

	{
	internal:
		SerialChunkReceiverWinRT(SerialChunkReceiver* pSocketChunkReceiver)
		{
			this->m_pSocketChunkReceiver = pSocketChunkReceiver;
		};
		SerialChunkReceiver* geSocketChunkReceiver() { return this->m_pSocketChunkReceiver; };

	internal:
		SerialChunkReceiver* m_pSocketChunkReceiver;
	};


    ref class SerialCom sealed
    {

		Concurrency::cancellation_token_source* m_cancellationTokenSource;
		bool m_acceptingData;
		Microsoft::WRL::Wrappers::SRWLock m_lock;
		unsigned int m_ReadBufferLen;
		std::list<SerialChunkReceiver*> * m_pSerialChunkConnectionList;

	public:
    SerialCom();
		virtual ~SerialCom();
		void SendDataToClients(Windows::Storage::Streams::IBuffer^ buffer);
 
	//	Windows::Foundation::IAsyncAction ^ SerialCom::conntectToDevice(Windows::Devices::Enumeration::DeviceInformation^ device);
		Windows::Foundation::IAsyncAction ^ SerialCom::conntectToDevice(Platform::String^ device);


		event Windows::Foundation::TypedEventHandler<Windows::Devices::SerialCommunication::SerialDevice ^, int>^ OnDeviceConnected; // for creation SocketChunkReceiver
    event Windows::Foundation::TypedEventHandler<Windows::Devices::SerialCommunication::SerialDevice ^, Platform::Exception ^>^ stopStreaming;
		event Windows::Foundation::TypedEventHandler<Platform::Object^, Platform::Exception ^>^ Failed;
    event Windows::Foundation::TypedEventHandler<Windows::Devices::SerialCommunication::SerialDevice ^, Windows::Devices::SerialCommunication::ErrorReceivedEventArgs ^ >^ onSerialErrorReceived;

		void CancelConnections();
		void DeleteAllConnections(void);


		SerialChunkReceiverWinRT^ AddChunkReceiver(SerialChunkReceiverWinRT^ pSocketChunkReceiver); // Add Communication Object for Serial Device

	private:

	

 //   Windows::Foundation::IAsyncOperation<Windows::Devices::Enumeration::DeviceInformationCollection ^> ^SerialCom::ListAvailableSerialDevicesAsync(void);

	//	Concurrency::task<void>  ConnectToSerialDeviceAsync(Windows::Devices::Enumeration::DeviceInformation^ device);
		Concurrency::task<void>  ConnectToSerialDeviceAsync(Platform::String^ device);

		bool DeleteConnectionBySerialDevice(Windows::Devices::SerialCommunication::SerialDevice ^serialPort);
		void OnStopStreaming(Windows::Devices::SerialCommunication::SerialDevice^ socket, Platform::Exception^ exception);
    void OnErrorReceived(Windows::Devices::SerialCommunication::SerialDevice ^sender, Windows::Devices::SerialCommunication::ErrorReceivedEventArgs ^args);
    };
}
