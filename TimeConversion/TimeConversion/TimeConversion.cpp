
#include "pch.h"
#include "TimeConversion.h"

#include <datetimeapi.h>

namespace ConversionHelpers
{
	TimeConversion::TimeConversion()
	{
	}


	TimeConversion::~TimeConversion()
	{
	}
	UINT64 TimeConversion::DeltaTime(const SYSTEMTIME st1, const SYSTEMTIME st2)
	{

		union timeunion { FILETIME fileTime; ULARGE_INTEGER ul; };
		timeunion ft1;
		timeunion ft2;
		SystemTimeToFileTime(&st1, &ft1.fileTime);
		SystemTimeToFileTime(&st2, &ft2.fileTime);

		return ft2.ul.QuadPart - ft1.ul.QuadPart;
	}

	UINT64 TimeConversion::FileTimeToMillis(const FILETIME &ft)
	{
		ULARGE_INTEGER uli;
		uli.LowPart = ft.dwLowDateTime; // could use memcpy here!
		uli.HighPart = ft.dwHighDateTime;

		return static_cast<UINT64>(uli.QuadPart / 10000);
	}
	
	UINT64 TimeConversion::getActualUnixTime()
	{
		SYSTEMTIME st;
		FILETIME ft;
		GetSystemTime(&st);
		SystemTimeToFileTime(&st, &ft);
		
		ULARGE_INTEGER uli;
		uli.LowPart = ft.dwLowDateTime; // could use memcpy here!
		uli.HighPart = ft.dwHighDateTime;

		UINT64 UnixTime =  static_cast<UINT64>(uli.QuadPart);

		return UnixTime;
	}

	void TimeConversion::NanosToSystemTime(UINT64 Nanos, SYSTEMTIME *st)
	{
		UINT64 multiplier = 1;
		UINT64 t = multiplier * Nanos;

		ULARGE_INTEGER li;
		li.QuadPart = t;
		// NOTE, DON'T have to do this any longer because we're putting
		// in the 64bit UINT directly
		//li.LowPart = static_cast<DWORD>(t & 0xFFFFFFFF);
		//li.HighPart = static_cast<DWORD>(t >> 32);

		FILETIME ft;
		ft.dwLowDateTime = li.LowPart;
		ft.dwHighDateTime = li.HighPart;

		::FileTimeToSystemTime(&ft, st);
	}

	void TimeConversion::NanosToLocalFileTime(UINT64 Nanos, FILETIME *st)
	{
		UINT64 multiplier = 1;
		UINT64 t = multiplier * Nanos;

		ULARGE_INTEGER li;
		li.QuadPart = t;
		// NOTE, DON'T have to do this any longer because we're putting
		// in the 64bit UINT directly
		//li.LowPart = static_cast<DWORD>(t & 0xFFFFFFFF);
		//li.HighPart = static_cast<DWORD>(t >> 32);

		FILETIME ft;
		ft.dwLowDateTime = li.LowPart;
		ft.dwHighDateTime = li.HighPart;

		FileTimeToLocalFileTime(&ft, st);


	}

	void TimeConversion::NanosToLocalSystemTime(UINT64 Nanos, SYSTEMTIME *st)
	{
		UINT64 multiplier = 1;
		UINT64 t = multiplier * Nanos;

		ULARGE_INTEGER li;
		li.QuadPart = t;
		// NOTE, DON'T have to do this any longer because we're putting
		// in the 64bit UINT directly
		//li.LowPart = static_cast<DWORD>(t & 0xFFFFFFFF);
		//li.HighPart = static_cast<DWORD>(t >> 32);

		FILETIME ft;
		ft.dwLowDateTime = li.LowPart;
		ft.dwHighDateTime = li.HighPart;

		FILETIME locft;
		FileTimeToLocalFileTime(&ft, &locft);

		::FileTimeToSystemTime(&locft, st);



	}

	void TimeConversion::ConvertFileTimeto_String(FILETIME* ftime, std::wstring& str) // ISO format, time zone designator Z == zero (UTC)
	{
		SYSTEMTIME SystemTime;
		FileTimeToSystemTime(ftime , &SystemTime)  ;

		wchar_t szLocalDate[255], szLocalTime[255];



		wchar_t buffer[200];



		GetDateFormatEx(LOCALE_NAME_SYSTEM_DEFAULT, DATE_SHORTDATE,  &SystemTime, NULL,szLocalDate, sizeof (szLocalDate) ,nullptr);

		GetTimeFormatEx(LOCALE_NAME_SYSTEM_DEFAULT, TIME_FORCE24HOURFORMAT, &SystemTime, NULL, szLocalTime, sizeof(szLocalTime));

		swprintf(&buffer[0], sizeof(buffer) / sizeof(buffer[0]), L"%s %s", szLocalDate, szLocalTime);

		str= buffer;


	}

	void TimeConversion::ConvertSystemTimeto_String(SYSTEMTIME* SystemTime, std::wstring& str) // ISO format, time zone designator Z == zero (UTC)
	{
		//SYSTEMTIME SystemTime;
		//FileTimeToSystemTime(ftime, &SystemTime);

		wchar_t szLocalDate[255], szLocalTime[255];



		wchar_t buffer[200];



		GetDateFormatEx(LOCALE_NAME_SYSTEM_DEFAULT, DATE_SHORTDATE, SystemTime, NULL, szLocalDate, sizeof(szLocalDate), nullptr);

		GetTimeFormatEx(LOCALE_NAME_SYSTEM_DEFAULT, TIME_FORCE24HOURFORMAT, SystemTime, NULL, szLocalTime, sizeof(szLocalTime));

		swprintf(&buffer[0], sizeof(buffer) / sizeof(buffer[0]), L"%s %s", szLocalDate, szLocalTime);

		str = buffer;


	}



	void TimeConversion::MillisToSystemTime(UINT64 millis, SYSTEMTIME *st)
	{
		UINT64 multiplier = 10000;
		UINT64 t = multiplier * millis;

		ULARGE_INTEGER li;
		li.QuadPart = t;
		// NOTE, DON'T have to do this any longer because we're putting
		// in the 64bit UINT directly
		//li.LowPart = static_cast<DWORD>(t & 0xFFFFFFFF);
		//li.HighPart = static_cast<DWORD>(t >> 32);

		FILETIME ft;
		ft.dwLowDateTime = li.LowPart;
		ft.dwHighDateTime = li.HighPart;

		::FileTimeToSystemTime(&ft, st);
	}


}
