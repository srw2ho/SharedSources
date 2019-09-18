#pragma once
#ifndef GPIOEVENTPACKAGEQUEUE_H_
#define GPIOEVENTPACKAGEQUEUE_H_

#include <queue>
#include <windows.h>
#include "GPIOEventPackageQueue.h"
#include "GPIODriverExportDefs.h"


#pragma pack(push, 1)	// byte oriented packed data

namespace GPIODriver
{
	class GPIODRIVER_EXPORT  GPIOEventPackage {

	private:
		int				 m_PinNo;
		Platform::String^ m_EventMsg;
		void*			m_pAdditional; // Additional Information
	public:
		GPIOEventPackage(void);
		GPIOEventPackage(int PinNo, Platform::String^ eventMessage, void* Additional=nullptr);
		virtual ~GPIOEventPackage();
		int getPinNo();
		Platform::String^ getEventMsg();
		void* getAdditional();

	};

	class GPIODRIVER_EXPORT GPIOEventPackageQueue

	{
	private:
		HANDLE m_hWriteEvent;

		std::vector<GPIOEventPackage*>* m_packetQueue;
		CRITICAL_SECTION m_CritLock;

	public:

		GPIOEventPackageQueue(void);
		virtual ~GPIOEventPackageQueue();

		virtual void cancelwaitForPacket();

		virtual void Flush();
		virtual void PushPacket(GPIOEventPackage* ppacket);

		virtual size_t waitForPacket(GPIOEventPackage** Packet, DWORD waitTime = INFINITE);

		virtual GPIOEventPackage* PopPacket();
		void Lock();

		void UnLock();
	};

}

#pragma pack(pop)
#endif