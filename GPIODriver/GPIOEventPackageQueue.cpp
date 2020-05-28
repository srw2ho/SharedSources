#include "pch.h"
#include "GPIODriver.h"
#include "GPIOEventPackageQueue.h"
#include "GPIOPin.h"



namespace GPIODriver
{

	GPIOEventPackage::GPIOEventPackage()
	{
		m_PinNo = -1;
	//	m_EventMsg = "";
		m_pAdditional = nullptr;
		m_buf = nullptr;

	}
	//GPIOEventPackage::GPIOEventPackage(int PinNo, Platform::String^ eventMessage, void* Additional)
	GPIOEventPackage::GPIOEventPackage(int PinNo, void* Additional)
	{
		m_PinNo = PinNo;
	//	m_EventMsg = eventMessage;
		m_pAdditional = Additional;
		m_buf = nullptr;
	
	}

	int GPIOEventPackage::getPinNo() { return m_PinNo; };

	//Platform::String^ GPIOEventPackage::getEventMsg() { return m_EventMsg; };
	Windows::Storage::Streams::IBuffer^ GPIOEventPackage::getEventBuffer() { return m_buf; };

	void GPIOEventPackage::setEventBuffer(Windows::Storage::Streams::IBuffer^ buf) {
		m_buf = buf;
	}

	void* GPIOEventPackage::getAdditional()
	{
		return m_pAdditional;
	}

	GPIOEventPackage::~GPIOEventPackage()
	{

	}

	GPIOEventPackageQueue::GPIOEventPackageQueue()
	{
		m_packetQueue = new std::vector<GPIOEventPackage*>();
		InitializeCriticalSection(&m_CritLock);
		m_hWriteEvent = CreateEvent(
			NULL,               // default security attributes
			TRUE,               // manual-reset event
			FALSE,              // initial state is nonsignaled
			nullptr
			//TEXT("WriteEvent")  // object name
		);
		m_UseMpack = false;
	}


	GPIOEventPackageQueue::~GPIOEventPackageQueue()
	{
		this->Flush();
		delete m_packetQueue;
		DeleteCriticalSection(&m_CritLock);
		CloseHandle(m_hWriteEvent);
	}

	void GPIOEventPackageQueue::cancelwaitForPacket() {
		::SetEvent(m_hWriteEvent);
	}

	void GPIOEventPackageQueue::Flush()
	{
		this->Lock();
		while (!m_packetQueue->empty())
		{
			GPIOEventPackage* Packet = PopPacket();
			delete Packet;
		}
		this->UnLock();
	};





	void GPIOEventPackageQueue::PushPacket(GPIOEventPackage* ppacket) {
		this->Lock();


		GPIOPin* pPin = (GPIOPin*)ppacket->getAdditional();
		if (pPin != nullptr) {
#if !GPIOCLIENT_USING
			pPin->setTimeinNanos(getActualTimeinNanos());
			if (m_UseMpack) {

				std::vector<char> data(200);
				size_t len = 0;
				pPin->doPack(&data[0], &len);

				if (len > 0) {
					data.resize(len);
					ppacket->setEventBuffer(createPayloadBufferfromMpackData(data));
				}
		}
			else {
				Platform::String^ message = pPin->GetGPIOPinCmd();

				if (message->Length() > 0) {
					// Sending to all connected clients
					ppacket->setEventBuffer(createPayloadBufferfromSendData(message));
				}
			}

#else

#endif


			m_packetQueue->push_back(ppacket);
			::SetEvent(m_hWriteEvent);

		}
		else {
			delete ppacket;
		}


		this->UnLock();

	};

	GPIOEventPackage* GPIOEventPackageQueue::PopPacket() {
		GPIOEventPackage*pPacketRet = nullptr;
		this->Lock();
		if (!m_packetQueue->empty())
		{
			pPacketRet = m_packetQueue->front();
			//	avPacket = m_packetQueue.front();
			m_packetQueue->erase(m_packetQueue->begin());

			//		m_packetQueue->pop();
			::ResetEvent(m_hWriteEvent);
		}
		this->UnLock();
		return pPacketRet;
	};


	size_t GPIOEventPackageQueue::waitForPacket(GPIOEventPackage** Packet, DWORD waitTime) {

		this->Lock();
		*Packet = nullptr;
		size_t size = m_packetQueue->size();
		if (size == 0) {//no packet then wait for
			this->UnLock();
			DWORD dwWaitResult = WaitForSingleObject(m_hWriteEvent, // event handle
				waitTime);    // indefinite wait
			if (dwWaitResult == WAIT_OBJECT_0) {}
			this->Lock();
			size = m_packetQueue->size();
			if (size > 0) {
				//	*Packet = m_packetQueue->at(size-1);
				//	DeleteFirstPackets(size - 2);
				*Packet = m_packetQueue->front();
				m_packetQueue->erase(m_packetQueue->begin());
			}
			::ResetEvent(m_hWriteEvent);
			this->UnLock();


		}
		else {
			*Packet = m_packetQueue->front();
			m_packetQueue->erase(m_packetQueue->begin());

			this->UnLock();
		}

		return size;
	};


	void GPIOEventPackageQueue::Lock() {
		EnterCriticalSection(&m_CritLock);
	}

	void GPIOEventPackageQueue::UnLock() {
		LeaveCriticalSection(&m_CritLock);
	}
}