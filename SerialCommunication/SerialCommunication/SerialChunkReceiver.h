#pragma once
#include <collection.h>
#include <ppltasks.h>
#include <wrl.h>
#include <robuffer.h>

#include "SerialEventSource.h"

using namespace concurrency;
using namespace Platform;

using namespace Windows::Devices::SerialCommunication;
using namespace Windows::Storage::Streams;

namespace SerialCommunication {

	class SerialChunkReceiver
	{
	protected:

		SerialCommunication::SerialEventSource ^ m_EventSource;

		concurrency::cancellation_token_source* m_pSocketCancelTaskToken;


		Windows::Devices::SerialCommunication::SerialDevice ^m_serialPort;

		Microsoft::WRL::Wrappers::SRWLock* m_plock;
		bool m_acceptingData;
		int m_ChunkProcessing;

		unsigned int m_chunkBufferSize;
		unsigned int m_ReadBufferLen;
		Windows::Storage::Streams::DataReader^ m_RecVReader;

	public:
		SerialChunkReceiver(Windows::Devices::SerialCommunication::SerialDevice ^_serialPort);
		virtual ~SerialChunkReceiver();

		virtual void StartService();
		virtual void Init();

		virtual void SendData(Windows::Storage::Streams::IBuffer^ buffer);
		virtual void CancelAsyncSocketOperation();

		SerialCommunication::SerialEventSource  ^ getSocketEventSource() { return m_EventSource; };
		Windows::Devices::SerialCommunication::SerialDevice^ getSerialDevice() { return m_serialPort; };

		virtual void clearRecvBuf();

	protected:
		virtual void ReceiveChunkLoop(
			Windows::Storage::Streams::DataReader^ reader,
			Windows::Devices::SerialCommunication::SerialDevice^ serialDev);
		virtual void DoProcessChunk(Windows::Storage::Streams::DataReader^ reader);
    virtual Windows::Storage::Streams::IBuffer^ getStartServiceCommand();
	};

}