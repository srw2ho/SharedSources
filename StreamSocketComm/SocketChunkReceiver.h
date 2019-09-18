#pragma once
#ifndef SOCKETCHUNK_RECEIVER_H_
#define SOCKETCHUNK_RECEIVER_H_
#include <collection.h>
#include <ppltasks.h>
#include <wrl.h>
#include <robuffer.h>

#include "SocketHelpers.h"

#include "SocketEventSource.h"
//#include "StreamSocketCommExports.h"

namespace StreamSocketComm
{



	//class STREAMSOCKETCOMM_EXPORT SocketChunkReceiver
	class  SocketChunkReceiver
	{

		concurrency::cancellation_token_source* m_pSocketCancelTaskToken;


		StreamSocketComm::SocketEventSource ^ m_EventSource;
		Windows::Networking::Sockets::StreamSocket^ m_socket;

	protected:

		int m_ChunkProcessing;
		unsigned int m_chunkBufferSize;

		int64 m_StartChunkReceived;
		int64 m_DeltaChunkReceived;
		unsigned int m_ReadBufferLen;

		Microsoft::WRL::Wrappers::SRWLock* m_plock;

	protected:
		bool m_acceptingData;

	public:
		SocketChunkReceiver(Windows::Networking::Sockets::StreamSocket^);
		virtual ~SocketChunkReceiver();

		virtual Platform::String^ getStartServiceString();
		virtual void StartService();
		virtual void Init();
		virtual void ReceiveChunkLoop(
			Windows::Storage::Streams::DataReader^ reader,
			Windows::Networking::Sockets::StreamSocket^ socket);

		virtual void CancelAsyncSocketOperation();

		virtual void SendData(Windows::Storage::Streams::IBuffer^ buffer);

		StreamSocketComm::SocketEventSource ^ getSocketEventSource() { return m_EventSource; };

		Windows::Networking::Sockets::StreamSocket^ getSocket() { return m_socket; };

		bool getAcceptingData() {return m_acceptingData;}
	protected:
		virtual void DoProcessChunk(Windows::Storage::Streams::DataReader^ reader);
	//	virtual Windows::Storage::Streams::IBuffer^  createBufferfromSendData(std::string& stringinfo);
	//	Platform::String^ StringFromAscIIChars(char* chars);

	};




}
#endif /**/