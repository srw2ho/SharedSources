#pragma once
#include <collection.h>
#include <ppltasks.h>
#include <wrl.h>
#include <robuffer.h>

namespace SerialCommunication
{

	class SerialHelpers
	{
	public:
		SerialHelpers();
		virtual ~SerialHelpers();

    static std::string convertString(const std::wstring& s);
		static Windows::Storage::Streams::IBuffer^  createBufferfromSendData(std::string& stringinfo);

		static Windows::Storage::Streams::IBuffer^ createBufferfromSendData(Platform::String^ stringinfo);
		static Windows::Storage::Streams::IBuffer^ createPayloadBufferfromSendData(Platform::String^ stringinfo);

		static Platform::String^ StringFromAscIIChars(const char* chars);

	};

}