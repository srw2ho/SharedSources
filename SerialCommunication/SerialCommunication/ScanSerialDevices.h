#pragma once

namespace SerialCommunication
{


  ref class ScanSerialDevices sealed
  {
  //  Platform::Collections::Vector<Platform::Object^>^ m_availableDevices;
  //  Platform::Collections::Vector<SerialCommunication::SerDevice^>^ m_availableDevices;
    Windows::Foundation::Collections::IObservableVector<Windows::Devices::Enumeration::DeviceInformation^>^ m_availableDevices;


  public:
    ScanSerialDevices();
    virtual ~ScanSerialDevices();

    property Windows::Foundation::Collections::IObservableVector<Windows::Devices::Enumeration::DeviceInformation^>^ AvailableDevices {

      Windows::Foundation::Collections::IObservableVector <Windows::Devices::Enumeration::DeviceInformation^>^ AvailableDevices::get() { return m_availableDevices; };

    }
    Windows::Foundation::IAsyncAction ^ readAvailableDevices(void);
    Windows::Foundation::IAsyncOperation<Windows::Devices::Enumeration::DeviceInformationCollection ^> ^ ListAvailableSerialDevicesAsync(void);

  protected:

  //  Windows::Foundation::IAsyncOperation<Windows::Devices::Enumeration::DeviceInformationCollection ^> ^ ListAvailableSerialDevicesAsync(void);

  };

}