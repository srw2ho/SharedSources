#pragma once
namespace SerialCommunication
{

	enum  NotificationEventReason {
		EndRequest = 0,
		CanceledbyUser,
		CanceledbyError
	};



	delegate void NotificationEventhandler(Windows::Devices::SerialCommunication::SerialDevice ^_serialPort, Platform::Exception^ exception);

	// interface that has an event and a function to invoke the event  
	interface struct EventInterface {
	public:
		event NotificationEventhandler ^ eventhandler;
		void fire(Windows::Devices::SerialCommunication::SerialDevice ^_serialPort, int status, Platform::Exception^ exception);
	};


	// class that implements the interface event and function  
	ref class SerialEventSource : public EventInterface {
	public:

		virtual event NotificationEventhandler^ eventhandler;

		virtual void fire(Windows::Devices::SerialCommunication::SerialDevice ^_serialPort, int status, Platform::Exception^ exception)
		{
			eventhandler(_serialPort, exception);

		}
	};

}