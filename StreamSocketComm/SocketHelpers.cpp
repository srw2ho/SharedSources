#include "pch.h"
#include "SocketHelpers.h"

using namespace Windows::Networking;
using namespace Windows::Networking::Sockets;
using namespace Windows::Storage::Streams;

namespace StreamSocketComm
{


	SocketHelpers::SocketHelpers()
	{
	}


	SocketHelpers::~SocketHelpers()
	{
	}

	Windows::Storage::Streams::IBuffer^ SocketHelpers::createBufferfromSendData(std::string& stringinfo) {


		DataWriter^ writer = ref new DataWriter();
		// Write first the length of the string a UINT32 value followed up by the string. The operation will just store 
		// the data locally.
		Platform::String^ stringToSend = SocketHelpers::StringFromAscIIChars((char*)stringinfo.c_str());

		writer->WriteUInt32(writer->MeasureString(stringToSend));
		writer->WriteByte(0x55);	// CheckByte 1 for verification
		writer->WriteByte(0x55);	// CheckByte 2
		writer->WriteString(stringToSend);

		return writer->DetachBuffer();

	}

	Windows::Storage::Streams::IBuffer^ SocketHelpers::createBufferfromSendData(Platform::String^ stringinfo) {


		DataWriter^ writer = ref new DataWriter();
		// Write first the length of the string a UINT32 value followed up by the string. The operation will just store 
		// the data locally.
	
		writer->WriteUInt32(writer->MeasureString(stringinfo));
		writer->WriteByte(0x55);	// CheckByte 1 for verification
		writer->WriteByte(0x55);	// CheckByte 2
		writer->WriteString(stringinfo);

		return writer->DetachBuffer();

	}

	Windows::Storage::Streams::IBuffer^ SocketHelpers::createPayloadBufferfromMpackData(std::vector<char>& mpackdata) {


		DataWriter^ writer = ref new DataWriter();
		// Write first the length of the string a UINT32 value followed up by the string. The operation will just store 
		// the data locally.

		writer->WriteUInt32(mpackdata.size());
		writer->WriteByte(0x55);	// CheckByte 1 for verification
		writer->WriteByte(0x51);	// CheckByte 2 for Payload

		Platform::Array<byte>^ arr = ref new Platform::Array<byte>((byte*)&mpackdata[0], mpackdata.size());
		writer->WriteBytes(arr);

		return writer->DetachBuffer();


	}



	Windows::Storage::Streams::IBuffer^ SocketHelpers::createPayloadBufferfromSendData(Platform::String^ stringinfo) {


		DataWriter^ writer = ref new DataWriter();
		// Write first the length of the string a UINT32 value followed up by the string. The operation will just store 
		// the data locally.

		writer->WriteUInt32(writer->MeasureString(stringinfo));
		writer->WriteByte(0x55);	// CheckByte 1 for verification
		writer->WriteByte(0x50);	// CheckByte 2 for Payload
		writer->WriteString(stringinfo);

		return writer->DetachBuffer();

	}

	Platform::String^ SocketHelpers::StringFromAscIIChars(char* chars)
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