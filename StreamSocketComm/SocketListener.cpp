#include "pch.h"
#include "SocketListener.h"


using namespace concurrency;

using namespace Platform;
using namespace Platform::Collections;
using namespace std;
using namespace Windows::Foundation;
using namespace Windows::Media::Capture;
using namespace Windows::Networking;
using namespace Windows::Networking::Connectivity;
using namespace Windows::Networking::Sockets;
using namespace Windows::Storage::Streams;
using namespace Windows::UI::Core;

namespace StreamSocketComm
{



	SocketListener::SocketListener()
	{
		m_listener = ref new Windows::Networking::Sockets::StreamSocketListener();
		m_listener->Control->KeepAlive = true;
		m_pSocketChunkConnectionList = new std::list<SocketChunkReceiver*>();
	}


	SocketListener::~SocketListener()
	{
		DeleteAllConnections();
		delete m_pSocketChunkConnectionList;
		delete m_listener;
	}

	void SocketListener::StartListener()
	{

		if (m_ChunkReceiverToken.Value == 0)
		{
			CancelConnections();


				m_ChunkReceiverToken = m_listener->ConnectionReceived +=
					ref new TypedEventHandler<StreamSocketListener^, StreamSocketListenerConnectionReceivedEventArgs^>(this, &SocketListener::OnConnectionReceived);


	
		}
	}

	void SocketListener::StopListener()
	{

		if (m_ChunkReceiverToken.Value > 0) {
			CancelConnections();
			m_listener->ConnectionReceived -= m_ChunkReceiverToken; //keine neue Connections zulassen
			m_ChunkReceiverToken.Value = 0;
		}


	}

	void SocketListener::CancelConnections()
	{
		auto lock = m_lock.LockExclusive();

		// cancel all pending Connections from getted Type
		while (!m_pSocketChunkConnectionList->empty())
		{
			SocketChunkReceiver*pConn = m_pSocketChunkConnectionList->back();
			m_pSocketChunkConnectionList->pop_back();

			pConn->CancelAsyncSocketOperation();

		}
	//	m_connections.UnLock();
	}

	void SocketListener::DeleteAllConnections(void) {
		auto lock = m_lock.LockExclusive();

		while (!m_pSocketChunkConnectionList->empty())
		{
			SocketChunkReceiver*pConn = m_pSocketChunkConnectionList->back();
			m_pSocketChunkConnectionList->pop_back();
			delete pConn;

		}


	}


	bool SocketListener::DeleteConnectionBySocket(Windows::Networking::Sockets::StreamSocket^ socket) {
		bool found = false;
		auto lock = m_lock.LockExclusive();
		std::list<SocketChunkReceiver*>::iterator it;

		for (it = m_pSocketChunkConnectionList->begin(); it != m_pSocketChunkConnectionList->end(); it++)
		{
			SocketChunkReceiver*pConn = *it;
			if (pConn->getSocket() == socket)
			{
				m_pSocketChunkConnectionList->remove(pConn);
				delete pConn;
				found = true;
				break;
			}

		}

		return found;

	}

	IAsyncAction ^ SocketListener::StartClientAsync(Platform::String^ hostNameForConnect, int port)
	{

		Platform::String^ servicename = port.ToString();

		auto server = this;

		return create_async([this, server, hostNameForConnect, servicename]()
			->void {

			//Trace("@%p creating CameraServer on adress %s:%s with camera", (void*)server, hostNameForConnect, servicename);
			CancelConnections();

			HostName^ hostName;
			try
			{
				hostName = ref new HostName(hostNameForConnect);
			}
			catch (InvalidArgumentException^ e)
			{

			}

			StreamSocket^ socket = ref new StreamSocket();


		//	auto token = this->m_pAsyncClientCancelTaskToken->get_token();
			socket->Control->KeepAlive = true;

			{
				create_task(socket->ConnectAsync(
					hostName,
					servicename,
					SocketProtectionLevel::PlainSocket)).then([this, server, socket](task<void> previousTask)
				{
					try
					{
						previousTask.get();

						this->startStreaming(this, socket); // Callback to MediaPlayder Source
						this->OnClientConnected(socket,0);
						/*
				//		GPIOServiceConnection* pConn = new GPIOServiceConnection(socket); // Connection from Type MepgVideo Client
						pConn = m_connections.AddConnection(pConn);
						if (pConn != nullptr) {
							pConn->getSocketEventSource()->eventhandler += ref new OpenCVFFMpegLib::NotificationEventhandler(this, &GPIOServiceClient::OnNotifiationEventhandler);
							pConn->StartService(); // Starte die Kommunikation und warte auf Socket-Close
						}
						*/

					}
					catch (task_canceled const &) // cancel by User
					{
						Failed(socket, nullptr);
						delete socket;
					}

					catch (Exception^ exception)
					{
						Failed(socket, exception);
						delete socket;

					}
				});
			}
		});

	}
	IAsyncAction ^ SocketListener::CreateListenerServerAsync(int port)
	{

		return create_async([this, port]()
		{

			StartListener();

			this->m_listener->Control->KeepAlive = false;


			return create_task(this->m_listener->BindServiceNameAsync(port == 0 ? L"" : port.ToString()))

				.then([this]() {

				this->m_port = _wtoi(this->m_listener->Information->LocalPort->Data());


				if (this->m_port == 0)
				{
					throw ref new InvalidArgumentException(L"Failed to convert TCP port");
				}
			

			});


		});


	}


	

	SocketChunkReceiverWinRT^ SocketListener::AddChunkReceiver(SocketChunkReceiverWinRT^ pSocketChunkReceiverWinRT)
	{
		SocketChunkReceiver*pSocketChunkReceiver = pSocketChunkReceiverWinRT->geSocketChunkReceiver();
		if (pSocketChunkReceiver == nullptr) return pSocketChunkReceiverWinRT;
			 
		pSocketChunkReceiver->getSocketEventSource()->eventhandler += ref new StreamSocketComm::NotificationEventhandler(this, &SocketListener::OnStopStreaming);
		pSocketChunkReceiver->Init();
		m_pSocketChunkConnectionList->push_back(pSocketChunkReceiver);

		return pSocketChunkReceiverWinRT;

	}
	

	void SocketListener::OnConnectionReceived(_In_ StreamSocketListener^ sender, _In_ StreamSocketListenerConnectionReceivedEventArgs^ e)
	{

		auto lock = m_lock.LockExclusive();

		StreamSocket^ socket = e->Socket;
		this->startStreaming(this, socket); // Callback to MediaPlayder Source
		this->OnCreateConnection(sender, e);

		/*
		DataReader^ reader = ref new DataReader(e->Socket->InputStream);
		reader->InputStreamOptions = InputStreamOptions::Partial;
		StreamSocket^ socket = e->Socket;

		this->startStreaming(this, socket); // Callback to MediaPlayder Source


		SocketChunkReceiver* pSocketChunkReceiver = new SocketChunkReceiver(socket);
		pSocketChunkReceiver->getSocketEventSource()->eventhandler += ref new StreamSocketComm::NotificationEventhandler(this, &SocketListener::OnStopStreaming);
		pSocketChunkReceiver->Init();
		pSocketChunkReceiver->ReceiveChunkLoop(reader, e->Socket);
		m_pSocketChunkConnectionList->push_back(pSocketChunkReceiver);
		*/


	}

	void SocketListener::OnStopStreaming(Windows::Networking::Sockets::StreamSocket^ socket, Platform::Exception^ exception)
	{

		DeleteConnectionBySocket(socket);
		this->stopStreaming(this,  exception); // Callback to MediaPlayder Source


	}

	void SocketListener::SendDataToClients(Windows::Storage::Streams::IBuffer^ buffer)
	{

		auto lock = m_lock.LockExclusive();

		std::list<SocketChunkReceiver*>::iterator it;
		for (it = m_pSocketChunkConnectionList->begin(); it != m_pSocketChunkConnectionList->end(); it++)
		{
			SocketChunkReceiver*pConn = *it;
			pConn->SendData(buffer);
		}


	}

}