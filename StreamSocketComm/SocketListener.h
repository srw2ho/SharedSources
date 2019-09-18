#pragma once
#ifndef SOCKETLISTENER_H_
#define SOCKETLISTENER_H_

#include <vector>
#include <list>
#include <memory>
#include <sstream>

#include <collection.h>
#include <ppltasks.h>
#include <wrl.h>
#include <robuffer.h>
#include <SocketHelpers.h>
#include <SocketChunkReceiver.h>

namespace StreamSocketComm
{
		
	ref class SocketChunkReceiverWinRT sealed

	{
	internal:
		SocketChunkReceiverWinRT(SocketChunkReceiver* pSocketChunkReceiver) 
		{
			this->m_pSocketChunkReceiver = pSocketChunkReceiver;
		};
		 SocketChunkReceiver* geSocketChunkReceiver()  { return this->m_pSocketChunkReceiver; };

	internal:
		SocketChunkReceiver* m_pSocketChunkReceiver;
	} ;
	

	ref class SocketListener sealed

	{
		Windows::Networking::Sockets::StreamSocketListener^ m_listener;
		int m_port;

		Windows::Foundation::EventRegistrationToken m_ChunkReceiverToken;

		Windows::Foundation::EventRegistrationToken m_BME280ChunkReceiveToken;

		std::list<SocketChunkReceiver*> * m_pSocketChunkConnectionList;
		Microsoft::WRL::Wrappers::SRWLock m_lock;


	public:
		SocketListener();
		virtual ~SocketListener();
	
	public:

		Windows::Foundation::IAsyncAction ^ CreateListenerServerAsync(int port);

		Windows::Foundation::IAsyncAction ^ StartClientAsync(Platform::String^ hostNameForConnect, int port);


		void CancelConnections();
		void DeleteAllConnections(void);
		
		event Windows::Foundation::TypedEventHandler<Platform::Object^, Windows::Networking::Sockets::StreamSocket^  >^ startStreaming;
		event Windows::Foundation::TypedEventHandler<Platform::Object^, Platform::Exception ^>^ stopStreaming;
		event Windows::Foundation::TypedEventHandler<Platform::Object^, Platform::Exception ^>^ Failed;

		event Windows::Foundation::TypedEventHandler<Windows::Networking::Sockets::StreamSocketListener^, Windows::Networking::Sockets::StreamSocketListenerConnectionReceivedEventArgs^>^ OnCreateConnection; // for creation SocketChunkReceiver
		event Windows::Foundation::TypedEventHandler< Windows::Networking::Sockets::StreamSocket^ , int>^ OnClientConnected; // for creation SocketChunkReceiver


		

		void SendDataToClients(Windows::Storage::Streams::IBuffer^ buffer);

		SocketChunkReceiverWinRT^ AddChunkReceiver(SocketChunkReceiverWinRT^ pSocketChunkReceiver);

//		SocketChunkReceiverWinRT^ AddChunkReceiver(SocketChunkReceiverWinRT^ pSocketChunkReceiver);

	protected:
		void StartListener();
		void StopListener();

		void OnConnectionReceived(
			_In_ Windows::Networking::Sockets::StreamSocketListener^ sender,
			_In_ Windows::Networking::Sockets::StreamSocketListenerConnectionReceivedEventArgs^ e
		);

		void OnStopStreaming(Windows::Networking::Sockets::StreamSocket^ socket, Platform::Exception^ exception);
		bool DeleteConnectionBySocket(Windows::Networking::Sockets::StreamSocket^ socket);
	};



}
#endif /**/