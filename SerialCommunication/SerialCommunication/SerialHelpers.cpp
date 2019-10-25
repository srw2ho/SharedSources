#include "pch.h"
#include "SerialHelpers.h"

using namespace Windows::Networking;
using namespace Windows::Devices::SerialCommunication;
using namespace Windows::Storage::Streams;

namespace SerialCommunication
{

	SerialHelpers::SerialHelpers()
	{
	}


	SerialHelpers::~SerialHelpers()
	{
	}

  std::string SerialHelpers::convertString(const std::wstring& s)
  {
    int len;
    int slength = (int)s.length() + 1;
    len = WideCharToMultiByte(CP_ACP, 0, s.c_str(), slength, 0, 0, 0, 0);
    std::string r(len, '\0');
    WideCharToMultiByte(CP_ACP, 0, s.c_str(), slength, &r[0], len, 0, 0);
    return r;
  }

	Windows::Storage::Streams::IBuffer^ SerialHelpers::createBufferfromSendData(std::string& stringinfo) {


		DataWriter^ writer = ref new DataWriter();
		// Write first the length of the string a UINT32 value followed up by the string. The operation will just store 
		// the data locally.
		Platform::String^ stringToSend = SerialHelpers::StringFromAscIIChars((char*)stringinfo.c_str());

		writer->WriteUInt32(writer->MeasureString(stringToSend));
		writer->WriteByte(0x55);	// CheckByte 1 for verification
		writer->WriteByte(0x55);	// CheckByte 2
		writer->WriteString(stringToSend);

		return writer->DetachBuffer();

	}

	Windows::Storage::Streams::IBuffer^ SerialHelpers::createBufferfromSendData(Platform::String^ stringinfo) {


		DataWriter^ writer = ref new DataWriter();
		// Write first the length of the string a UINT32 value followed up by the string. The operation will just store 
		// the data locally.

		writer->WriteUInt32(writer->MeasureString(stringinfo));
		writer->WriteByte(0x55);	// CheckByte 1 for verification
		writer->WriteByte(0x55);	// CheckByte 2
		writer->WriteString(stringinfo);

		return writer->DetachBuffer();

	}

	Windows::Storage::Streams::IBuffer^ SerialHelpers::createPayloadBufferfromSendData(Platform::String^ stringinfo) {


		DataWriter^ writer = ref new DataWriter();
		// Write first the length of the string a UINT32 value followed up by the string. The operation will just store 
		// the data locally.

		writer->WriteUInt32(writer->MeasureString(stringinfo));
		writer->WriteByte(0x55);	// CheckByte 1 for verification
		writer->WriteByte(0x50);	// CheckByte 2 for Payload
		writer->WriteString(stringinfo);

		return writer->DetachBuffer();

	}

	Platform::String^ SerialHelpers::StringFromAscIIChars(const char* chars)
	{
		size_t newsize = strlen(chars) + 1;
		wchar_t * wcstring = new wchar_t[newsize];
		size_t convertedChars = 0;
		mbstowcs_s(&convertedChars, wcstring, newsize, chars, _TRUNCATE);
		Platform::String^ str = ref new Platform::String(wcstring);
		delete[] wcstring;
		return str;
	}
}
