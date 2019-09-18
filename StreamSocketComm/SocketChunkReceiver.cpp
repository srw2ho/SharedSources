#include "pch.h"
#include "SocketHelpers.h"
#include "SocketChunkReceiver.h"
#include <ppl.h>



using namespace concurrency;
using namespace Platform;
using namespace Windows::Networking;
using namespace Windows::Networking::Sockets;
using namespace Windows::Storage::Streams;

namespace StreamSocketComm
{

	SocketChunkReceiver::SocketChunkReceiver(Windows::Networking::Sockets::StreamSocket^socket)
	{
		m_EventSource = ref new SocketEventSource();
		m_socket = socket;
		m_acceptingData = false;
		m_pSocketCancelTaskToken = new concurrency::cancellation_token_source();
		m_plock = new Microsoft::WRL::Wrappers::SRWLock();
		m_ReadBufferLen = 640 * 500;
		m_StartChunkReceived = 0;
		m_DeltaChunkReceived = 0;

	}


	SocketChunkReceiver::~SocketChunkReceiver()
	{
		if (m_socket != nullptr) {

			delete m_socket;
			m_socket = nullptr;
		}
		delete m_pSocketCancelTaskToken;
		delete m_plock;
	}

	void SocketChunkReceiver::Init()
	{
	
		m_acceptingData = false;
		m_ChunkProcessing = 0;
		m_StartChunkReceived = 0;
		m_DeltaChunkReceived = 0;
		m_ReadBufferLen = 640 * 500;

	}

	void SocketChunkReceiver::CancelAsyncSocketOperation() {
		m_acceptingData = false;
		m_pSocketCancelTaskToken->cancel();


	}


	void SocketChunkReceiver::SendData(Windows::Storage::Streams::IBuffer^ buffer)
	{
	

		if (buffer==nullptr)	return;

		if (buffer->Length == 0) return;

		if ( !m_acceptingData)
		{
			return;
		}

		auto lock = m_plock->LockExclusive();

	//	Windows::Storage::Streams::IBuffer^ buffer = createBufferfromSendData(info);

		auto token = m_pSocketCancelTaskToken->get_token();
		// Write the locally buffered data to the network.

		auto StoreAsync = m_socket->OutputStream->WriteAsync(buffer);

		create_task(StoreAsync).then([this](unsigned int bytesStored)
		{
			if (bytesStored > 0) {
				//	Trace("@%p completed sending HTTP part @%p", (void*)this, (void*)buffer);

				auto lock = m_plock->LockExclusive();
				return m_socket->OutputStream->FlushAsync();
			}
			else
			{ // when nothing is sended -> cancel
	
				cancel_current_task();
			}


		}, token).then([this](task<bool> previos) {
			bool bflushed = false;
			try
			{
				bflushed = previos.get();
			}

			catch (task_canceled const &) // cancel by User
			{
				return;
			}
			catch (Exception^ exception) {
				return;
			}

			catch (...)
			{
				return;
			}

		});
	}

	/*
	Windows::Storage::Streams::IBuffer^ SocketChunkReceiver::createBufferfromSendData(std::string& stringinfo) {


		DataWriter^ writer = ref new DataWriter();
		// Write first the length of the string a UINT32 value followed up by the string. The operation will just store 
		// the data locally.
		Platform::String^ stringToSend = StringFromAscIIChars((char*)stringinfo.c_str());

		writer->WriteUInt32(writer->MeasureString(stringToSend));
		writer->WriteByte(0x55);	// CheckByte 1 for verification
		writer->WriteByte(0x55);	// CheckByte 2
		writer->WriteString(stringToSend);

		return writer->DetachBuffer();

	}
	*/

	void SocketChunkReceiver::DoProcessChunk(DataReader^ reader)
	{
		bool doReadBuffer = false;

		unsigned int nread = 6;
		if (reader->UnconsumedBufferLength > nread)
			// 4 bytes for length and 2 bytes for verifikation
		{
			unsigned int readBufferLen = reader->ReadUInt32();

			byte checkByte1 = reader->ReadByte();
			byte checkByte2 = reader->ReadByte();

			if ((checkByte1 == 0x55) && (checkByte2 == 0x55)) { // prufen, ob stream richtig ist
			// Do Processing - Message

				unsigned int _len = reader->UnconsumedBufferLength;

				if (reader->UnconsumedBufferLength < readBufferLen) doReadBuffer = true;
				else {
					Platform::String^  rec = reader->ReadString(readBufferLen);
					if (rec == ("BME280Server.Start")) {
						std::string command = "BME280Server.Started";
						Windows::Storage::Streams::IBuffer^ buf = SocketHelpers::createBufferfromSendData(command);
						m_acceptingData = true;
						this->SendData(buf);

					}
					else if (rec == ("BME280Server.Stop")) {
						std::string command = "BME280Server.Stopped";
						Windows::Storage::Streams::IBuffer^ buf = SocketHelpers::createBufferfromSendData(command);
						m_acceptingData = false;
						this->SendData(buf);
					}
					else
					{
						//		DoProcessGPIOCommand(rec);
					}

				}
			}
			else doReadBuffer = true;
			if (doReadBuffer)
			{ // alles auslesen
				IBuffer^  chunkBuffer = reader->ReadBuffer(reader->UnconsumedBufferLength);
			}

		}

	}

	void SocketChunkReceiver::ReceiveChunkLoop(Windows::Storage::Streams::DataReader^ reader, Windows::Networking::Sockets::StreamSocket^ socket)
	{
		// Read first 4 bytes (length of the subsequent string).
		auto token = m_pSocketCancelTaskToken->get_token();
		unsigned int readBufferLen = m_ReadBufferLen;
		create_task(reader->LoadAsync(readBufferLen), token).then([this, reader, socket](task<unsigned int> size) {


			unsigned int readlen = 0;
			readlen = size.get();
			if (readlen == 0) {
				cancel_current_task();
			}
			else {
				DoProcessChunk(reader);
			}


		}).then([this, reader, socket](task<void> previous) {

			try {
				previous.get();
				ReceiveChunkLoop(reader, socket);
			}
			catch (task_canceled&)
			{
				auto lock = m_plock->LockExclusive();
				m_acceptingData = false;
				this->getSocketEventSource()->fire(socket, StreamSocketComm::NotificationEventReason::CanceledbyUser,nullptr);
	//			delete socket;
	//			m_socket = nullptr;

			}
			catch (Exception^ exception) {
				auto lock = m_plock->LockExclusive();
				m_acceptingData = false;
				this->getSocketEventSource()->fire(socket, StreamSocketComm::NotificationEventReason::CanceledbyError, exception);
	//			delete socket;
	//			m_socket = nullptr;
			}
			/*
			catch (...)
			{
				auto lock = m_plock->LockExclusive();
				m_acceptingData = false;
				this->getSocketEventSource()->fire(socket, StreamSocketComm::NotificationEventReason::CanceledbyError, exception);
//				delete socket;
//				m_socket = nullptr;

			}
			*/
		});



	}




	Platform::String^ SocketChunkReceiver::getStartServiceString() {

		Platform::String^ stringToSend = "BME280Server.Start";
		return stringToSend;
	}


	void SocketChunkReceiver::StartService()

	{
		StreamSocket^ socket = m_socket;
		//NT_ASSERT(socket != nullptr);
		auto token = m_pSocketCancelTaskToken->get_token();

	//	std::string stringToSend = "BME280Server.Start";
		Platform::String^ stringToSend = getStartServiceString();

		Windows::Storage::Streams::IBuffer^ buffer = SocketHelpers::createBufferfromSendData(stringToSend);

		// Write the locally buffered data to the network.

		auto WriteAsync = socket->OutputStream->WriteAsync(buffer);

		create_task(WriteAsync).then([this, socket](unsigned int) {

			return socket->OutputStream->FlushAsync();

		}, token).then([this, socket](bool ret)
		{
			// Reveive all Data from Client
			DataReader^ reader = ref new ::Windows::Storage::Streams::DataReader(socket->InputStream);
			reader->InputStreamOptions = InputStreamOptions::Partial;
			m_acceptingData = true;
			ReceiveChunkLoop(reader, socket);

		});

		return;

	}
	/*
	Platform::String^ SocketChunkReceiver::StringFromAscIIChars(char* chars)
	{
		size_t newsize = strlen(chars) + 1;
		wchar_t * wcstring = new wchar_t[newsize];
		size_t convertedChars = 0;
		mbstowcs_s(&convertedChars, wcstring, newsize, chars, _TRUNCATE);
		Platform::String^ str = ref new Platform::String(wcstring);
		delete[] wcstring;
		return str;
	}
	*/
}