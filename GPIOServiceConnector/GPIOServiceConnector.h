#pragma once

#include <queue>
#include <map>

#include "SocketListener.h"
#include "GPIOInOut.h"


typedef std::map<int, GPIODriver::GPIOInputPin* > MapInputClientsPins;

typedef std::map<Platform::String^, GPIODriver::GPIOPin* > MapClientsPins;

namespace GPIOServiceConnector
{
    public ref class GPIOConnector sealed
    {
		StreamSocketComm::SocketListener^  m_pSocketListener;
		concurrency::cancellation_token_source* m_pPackageCancelTaskToken;

		GPIODriver::GPIOInOut^ m_GPIOClientInOut;
		GPIODriver::GPIOs  m_Outputs;
		GPIODriver::GPIOEventPackageQueue* m_pGPIOEventPackageQueue;

		bool m_bProcessingPackagesStarted;
		concurrency::task<void> m_ProcessingPackagesTsk;
		Windows::Foundation::EventRegistrationToken m_startStreamingEventRegister;
		Windows::Foundation::EventRegistrationToken m_stopStreamingEventRegister;
		Windows::Foundation::EventRegistrationToken m_FailedEventRegister;
		Windows::Foundation::EventRegistrationToken m_OnClientConnected;
		Windows::Foundation::EventRegistrationToken m_InputConfigOptionsMapChanged;
		Platform::String ^ m_HostName;
		int m_port;
		unsigned int m_FailedConnectionCount;
		Windows::Foundation::Collections::IPropertySet^ m_outputconfigoptions;
		Windows::Foundation::Collections::IPropertySet^ m_inputconfigoptions;
	
    public:
		GPIOConnector();
		virtual ~GPIOConnector();
		event Windows::Foundation::TypedEventHandler<Platform::Object^, Windows::Foundation::Collections::IPropertySet^  >^ ChangeGPIOs;

		event Windows::Foundation::TypedEventHandler<Platform::Object^, Windows::Networking::Sockets::StreamSocket^  >^ startStreaming;
		event Windows::Foundation::TypedEventHandler<Platform::Object^, Platform::String ^>^ stopStreaming;
		event Windows::Foundation::TypedEventHandler<Platform::Object^, Platform::String ^> ^ Failed;
		Windows::Foundation::IAsyncAction ^ startProcessingPackagesAsync(Windows::Foundation::Collections::IPropertySet^ inputconfigoptions, Windows::Foundation::Collections::IPropertySet^ outputconfigoptions);
		Windows::Foundation::IAsyncAction ^ stopProcessingPackagesAsync();

		// Using  of MSGPack yes not
		property bool UseMsgPack {
			bool get();
			void set(bool value);
		}


		property Platform::String ^ HostName {
			Platform::String ^  get() { return m_HostName; };
			void set(Platform::String ^ value);
		}

		property int Port {
			int  get() { return m_port; };
			void set(int value);
		}

		property unsigned int  FailedConnectionCount {
			unsigned int   get() { return m_FailedConnectionCount; };
			void set(unsigned int   value);
		}

	private:
		Concurrency::task<void> doProcessPackages();
		void cancelPackageAsync();
		void OnFailed(Platform::Object ^sender, Platform::Exception ^args);
		void OnOnClientConnected(Windows::Networking::Sockets::StreamSocket ^sender, int args);
		void OnstartStreaming(Platform::Object ^sender, Windows::Networking::Sockets::StreamSocket ^args);
		void OnstopStreaming(Platform::Object ^sender, Platform::Exception ^exception);
		bool InitGPIOOutput(Windows::Foundation::Collections::IPropertySet^ inputconfigoptions);
		void clearGPIOs();
		void startProcessingPackages(Windows::Foundation::Collections::IPropertySet^ inputconfigoptions, Windows::Foundation::Collections::IPropertySet^ outputconfigoptions);
		void stopProcessingPackages();
		void OnMapChanged(Windows::Foundation::Collections::IObservableMap<Platform::String ^, Platform::Object ^> ^sender, Windows::Foundation::Collections::IMapChangedEventArgs<Platform::String ^> ^event);
	};
}
