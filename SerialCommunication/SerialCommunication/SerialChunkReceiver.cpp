#include "pch.h"
#include "SerialHelpers.h"
#include "SerialChunkReceiver.h"



namespace SerialCommunication {

	SerialChunkReceiver::SerialChunkReceiver(Windows::Devices::SerialCommunication::SerialDevice ^_serialPort)
	{
		m_serialPort = _serialPort;
		m_acceptingData = false;
		m_pSocketCancelTaskToken = new concurrency::cancellation_token_source();
		m_plock = new Microsoft::WRL::Wrappers::SRWLock();

		m_EventSource = ref new SerialEventSource();

		m_acceptingData = false;
		m_ReadBufferLen = 100;
		m_RecVReader = nullptr;


	}


	SerialChunkReceiver::~SerialChunkReceiver()
	{
		if (m_serialPort != nullptr) {

			delete m_serialPort;
			m_serialPort = nullptr;
		}
		delete m_pSocketCancelTaskToken;
		delete m_plock;
	}

	void SerialChunkReceiver::DoProcessChunk(Windows::Storage::Streams::DataReader^ reader)
	{
		bool doReadBuffer = false;

		unsigned int nread = 6;
		if (reader->UnconsumedBufferLength > nread)
			// 4 bytes for length and 2 bytes for verifikation
		{
			unsigned int readBufferLen = reader->ReadUInt32();


		}

	}




	void SerialChunkReceiver::ReceiveChunkLoop(Windows::Storage::Streams::DataReader^ reader, Windows::Devices::SerialCommunication::SerialDevice^ serialDev)
	{
		// Read first 4 bytes (length of the subsequent string).
		auto token = m_pSocketCancelTaskToken->get_token();
		unsigned int readBufferLen = m_ReadBufferLen;
		create_task(reader->LoadAsync(readBufferLen), token).then([this, reader, serialDev](task<unsigned int> size) {


			unsigned int readlen = 0;
			readlen = size.get();
			if (readlen == 0) {
				cancel_current_task();
			}
			else {
				DoProcessChunk(reader);
			}


			}).then([this, reader, serialDev](task<void> previous) {

				try {
					previous.get();
					ReceiveChunkLoop(reader, serialDev);
				}
				catch (task_canceled&)
				{
					auto lock = m_plock->LockExclusive();
					m_acceptingData = false;
					this->getSocketEventSource()->fire(serialDev, SerialCommunication::NotificationEventReason::CanceledbyUser, nullptr);


				}
				catch (Exception^ exception) {
					auto lock = m_plock->LockExclusive();
					m_acceptingData = false;
					this->getSocketEventSource()->fire(serialDev, SerialCommunication::NotificationEventReason::CanceledbyError, exception);

				}

				});



	}


	void SerialChunkReceiver::SendData(Windows::Storage::Streams::IBuffer^ buffer)
	{


		if (buffer == nullptr)	return;

		if (buffer->Length == 0) return;

		if (!m_acceptingData)
		{
			return;
		}

		auto lock = m_plock->LockExclusive();

		//	Windows::Storage::Streams::IBuffer^ buffer = createBufferfromSendData(info);

		auto token = m_pSocketCancelTaskToken->get_token();


		auto StoreAsync = m_serialPort->OutputStream->WriteAsync(buffer);

		create_task(StoreAsync,token).then([this](task< unsigned int> previous)
			{
				try
				{
					unsigned int stored = previous.get();
					if (stored == 0) {
						bool ret = true;
					}
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

	Windows::Storage::Streams::IBuffer^ SerialChunkReceiver::getStartServiceCommand()
	{
		return nullptr;


	}

	void  SerialChunkReceiver::clearRecvBuf()
	{
		if (m_RecVReader == nullptr) return;

		while (m_RecVReader->UnconsumedBufferLength > 0) {
			m_RecVReader->ReadBuffer(m_RecVReader->UnconsumedBufferLength);;
		};

	}

	void SerialChunkReceiver::StartService()

	{
		SerialDevice^ socket = m_serialPort;
		//NT_ASSERT(socket != nullptr);
		auto token = m_pSocketCancelTaskToken->get_token();


		Windows::Storage::Streams::IBuffer^ buffer = getStartServiceCommand();


		auto WriteAsync = socket->OutputStream->WriteAsync(buffer);

		create_task(WriteAsync,token).then([this, socket](task <unsigned int> previous) {
			try {
				unsigned int sended = previous.get();
				// Reveive all Data from Client
				DataReader^ reader = ref new ::Windows::Storage::Streams::DataReader(socket->InputStream);
				reader->UnicodeEncoding = UnicodeEncoding::Utf8;
				reader->ByteOrder = ByteOrder::BigEndian;

				m_RecVReader = reader;
				reader->InputStreamOptions = InputStreamOptions::Partial;
				m_acceptingData = true;
				ReceiveChunkLoop(reader, socket);
			}
			catch (task_canceled&)
			{

			}
			catch (Exception^ exception) {

			}

			});



	}

	void SerialChunkReceiver::Init()
	{

		m_acceptingData = false;
		m_ChunkProcessing = 0;
		m_ReadBufferLen = 640;

	}
	void SerialChunkReceiver::CancelAsyncSocketOperation() {
		m_acceptingData = false;
		m_pSocketCancelTaskToken->cancel();


	}





}