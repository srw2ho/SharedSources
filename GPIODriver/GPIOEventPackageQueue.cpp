#include "pch.h"
#include "GPIODriver.h"
#include "GPIOEventPackageQueue.h"


namespace GPIODriver
{

	GPIOEventPackage::GPIOEventPackage()
	{
		m_PinNo = -1;
		m_EventMsg = "";
		m_pAdditional = nullptr;

	}
	GPIOEventPackage::GPIOEventPackage(int PinNo, Platform::String^ eventMessage, void* Additional)
	{
		m_PinNo = PinNo;
		m_EventMsg = eventMessage;
		m_pAdditional = Additional;
	}

	int GPIOEventPackage::getPinNo() { return m_PinNo; };

	Platform::String^ GPIOEventPackage::getEventMsg() { return m_EventMsg; };


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
		m_packetQueue->push_back(ppacket);
		::SetEvent(m_hWriteEvent);

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