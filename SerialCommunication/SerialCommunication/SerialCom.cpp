#include "pch.h"
//#include "SerDevice.h"
#include "SerialCom.h"

using namespace SerialCommunication;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Concurrency;
using namespace Platform;

SerialCom::SerialCom()
{

	m_cancellationTokenSource = nullptr;
	m_pSerialChunkConnectionList = new std::list<SerialChunkReceiver*>();

	// m_availableDevices = ref new Platform::Collections::Vector<Platform::Object^>();

}

SerialCom::~SerialCom()
{
	DeleteAllConnections();
	delete m_pSerialChunkConnectionList;

}

SerialChunkReceiverWinRT^ SerialCom::AddChunkReceiver(SerialChunkReceiverWinRT^ pSocketChunkReceiverWinRT)
{
	SerialChunkReceiver*pSocketChunkReceiver = pSocketChunkReceiverWinRT->geSocketChunkReceiver();
	if (pSocketChunkReceiver == nullptr) return pSocketChunkReceiverWinRT;

	pSocketChunkReceiver->getSocketEventSource()->eventhandler += ref new SerialCommunication::NotificationEventhandler(this, &SerialCom::OnStopStreaming);
	pSocketChunkReceiver->Init();
	m_pSerialChunkConnectionList->push_back(pSocketChunkReceiver);

	return pSocketChunkReceiverWinRT;

}



void SerialCom::SendDataToClients(Windows::Storage::Streams::IBuffer^ buffer)
{

	auto lock = m_lock.LockExclusive();

	std::list<SerialChunkReceiver*>::iterator it;
	for (it = m_pSerialChunkConnectionList->begin(); it != m_pSerialChunkConnectionList->end(); it++)
	{
		SerialChunkReceiver*pConn = *it;
		pConn->SendData(buffer);
	}


}


void SerialCom::CancelConnections()
{
	auto lock = m_lock.LockExclusive();

  std::list<SerialChunkReceiver*>::iterator it;

  for (it = m_pSerialChunkConnectionList->begin(); it != m_pSerialChunkConnectionList->end(); it++) {
    SerialChunkReceiver* pConn = *it;
    pConn->CancelAsyncSocketOperation();
  }
  /*
	// cancel all pending Connections from getted Type
	while (!m_pSerialChunkConnectionList->empty())
	{
		SerialChunkReceiver*pConn = m_pSerialChunkConnectionList->back();
		m_pSerialChunkConnectionList->pop_back();

		pConn->CancelAsyncSocketOperation();

	}
  */
}

void SerialCom::DeleteAllConnections(void) {
	auto lock = m_lock.LockExclusive();

	while (!m_pSerialChunkConnectionList->empty())
	{
		SerialChunkReceiver*pConn = m_pSerialChunkConnectionList->back();
		m_pSerialChunkConnectionList->pop_back();
		delete pConn;

	}


}
bool SerialCom::DeleteConnectionBySerialDevice(Windows::Devices::SerialCommunication::SerialDevice ^serialPort) {
	bool found = false;
	auto lock = m_lock.LockExclusive();
	std::list<SerialChunkReceiver*>::iterator it;

	for (it = m_pSerialChunkConnectionList->begin(); it != m_pSerialChunkConnectionList->end(); it++)
	{
		SerialChunkReceiver*pConn = *it;
		if (pConn->getSerialDevice() == serialPort)
		{
			m_pSerialChunkConnectionList->remove(pConn);
			delete pConn;
			found = true;
			break;
		}

	}

	return found;

}



//Windows::Foundation::IAsyncAction^ SerialCom::conntectToDevice(Windows::Devices::Enumeration::DeviceInformation^ device)
Windows::Foundation::IAsyncAction^ SerialCom::conntectToDevice(Platform::String^ device)
{

	return Concurrency::create_async([this, device]()
		->void {
			{
				auto tsks = ConnectToSerialDeviceAsync(device);
				//  tsks.wait();
			}

		});

}

Concurrency::task<void> SerialCom::ConnectToSerialDeviceAsync(Platform::String^device)
{

	//	auto childTokenSource = Concurrency::cancellation_token_source::create_linked_source(cancellationToken);
		//auto token = m_cancellationTokenSource->get_token();
	return Concurrency::create_task(Windows::Devices::SerialCommunication::SerialDevice::FromIdAsync(device))
		.then([this, device](task<Windows::Devices::SerialCommunication::SerialDevice ^> serial_deviceTsk)
			{
				try
				{

					Windows::Devices::SerialCommunication::SerialDevice ^serial_device = serial_deviceTsk.get();
          if (serial_device != nullptr) {
            serial_device->ErrorReceived += ref new Windows::Foundation::TypedEventHandler<Windows::Devices::SerialCommunication::SerialDevice^, Windows::Devices::SerialCommunication::ErrorReceivedEventArgs^>(this, &SerialCommunication::SerialCom::OnErrorReceived);

            this->OnDeviceConnected(serial_device, 0);
          }
          else {
            Platform::Exception^ ex = Exception::CreateException(E_NOTIMPL); // SerDevice is not implemented
            Failed(device, ex);
          }



				}
				catch (task_canceled const &) // cancel by User
				{
					Failed(device, nullptr);
					//		delete serial_device;
				}

				catch (Platform::Exception ^ex)
				{
					//status->Text = "Error connecting to device!\nsendTextButton_Click: " + ex->Message;
					// perform any cleanup needed
					Failed(device, ex);

				}
			});
}



void SerialCom::OnStopStreaming(Windows::Devices::SerialCommunication::SerialDevice^ socket, Platform::Exception^ exception) {

	this->stopStreaming(socket, exception); // Callback to listener

	DeleteConnectionBySerialDevice(socket);


}



void SerialCommunication::SerialCom::OnErrorReceived(Windows::Devices::SerialCommunication::SerialDevice ^sender, Windows::Devices::SerialCommunication::ErrorReceivedEventArgs ^args)
{

	this->onSerialErrorReceived(sender, args); // Callback to MediaPlayder Source
  /*
	args->Error
	  BufferOverrun
	  1
	  A character - buffer overrun has occurred.The next character is lost.
	  Frame
	  0
	  The hardware detected a framing error.
	  ReceiveFull
	  2
	  An input buffer overflow has occurred.There is either no room in the input buffer, or a character was received after the end - of - file(EOF) character.
	  ReceiveParity
	  3
	  The hardware detected a parity error.
	  TransmitFull
	  4
	  The application tried to transmit a character, but the output buffer was full.
  */
  // throw ref new Platform::NotImplementedException();
}
