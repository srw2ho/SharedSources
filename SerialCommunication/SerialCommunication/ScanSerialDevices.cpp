#include "pch.h"
#include "ScanSerialDevices.h"

namespace SerialCommunication
{

  using namespace Concurrency;
  using namespace Platform;

  ScanSerialDevices::ScanSerialDevices()
  {
    m_availableDevices = ref new Platform::Collections::Vector<Windows::Devices::Enumeration::DeviceInformation ^>();

  }


  ScanSerialDevices::~ScanSerialDevices()
  {
  }

  Windows::Foundation::IAsyncOperation<Windows::Devices::Enumeration::DeviceInformationCollection ^> ^ScanSerialDevices::ListAvailableSerialDevicesAsync(void)

  {

    // Construct AQS String for all serial devices on system

    Platform::String ^serialDevices_aqs = Windows::Devices::SerialCommunication::SerialDevice::GetDeviceSelector();


    // Identify all paired devices satisfying query

    return Windows::Devices::Enumeration::DeviceInformation::FindAllAsync(serialDevices_aqs);

  }

  Windows::Foundation::IAsyncAction ^ ScanSerialDevices::readAvailableDevices(void)
  {
    return Concurrency::create_async([this]()
      ->void {
        {

          Concurrency::create_task(ListAvailableSerialDevicesAsync()).then([this](task<Windows::Devices::Enumeration::DeviceInformationCollection ^ > serialDeviceCollectionTask)

            {
              try {
                Windows::Devices::Enumeration::DeviceInformationCollection ^_deviceCollection = serialDeviceCollectionTask.get();
                // start with an empty list

                m_availableDevices->Clear();


                for (auto device : _deviceCollection)

                {
                
                  m_availableDevices->Append(device);


                }



              }
              catch (Exception ^  ex) {

              }


            });

        }

      });

    //using asynchronous operation, get a list of serial devices available on this device



  }
}