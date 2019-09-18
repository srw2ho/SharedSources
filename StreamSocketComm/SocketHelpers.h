#pragma once
#ifndef SOCKETHELPER_H_
#define SOCKETHELPER_H_
#include <collection.h>
#include <ppltasks.h>
#include <wrl.h>
#include <robuffer.h>

namespace StreamSocketComm
{

	class SocketHelpers
	{
	public:
		SocketHelpers();
		~SocketHelpers();

		
		static Windows::Storage::Streams::IBuffer^  createBufferfromSendData(std::string& stringinfo);

		static Windows::Storage::Streams::IBuffer^ createBufferfromSendData(Platform::String^ stringinfo);
		static Windows::Storage::Streams::IBuffer^ createPayloadBufferfromSendData(Platform::String^ stringinfo);

		static Platform::String^ StringFromAscIIChars(char* chars);
	};

}

#endif /**/