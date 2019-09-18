#pragma once
#ifndef TIMECONVERSION_H_
#define TIMECONVERSION_H_


#include <string>
#include <windows.h>



namespace ConversionHelpers
{

	class  TimeConversion
	{
	public:
		TimeConversion();
		virtual ~TimeConversion();

		static UINT64 DeltaTime(const SYSTEMTIME st1, const SYSTEMTIME st2);



		static UINT64 FileTimeToMillis(const FILETIME &ft);

		static UINT64 getActualUnixTime();


		static void NanosToLocalFileTime(UINT64 Nanos, FILETIME *st);
		static void NanosToLocalSystemTime(UINT64 Nanos, SYSTEMTIME *st);
		static void NanosToSystemTime(UINT64 Nanos, SYSTEMTIME *st); // convert nano in system time
		static void MillisToSystemTime(UINT64 millis, SYSTEMTIME *st);

		static void ConvertFileTimeto_String(FILETIME* ftime, std::wstring& str);
		static void ConvertSystemTimeto_String(SYSTEMTIME* ftime, std::wstring& str);

	};
}

#endif//