#pragma once

#include <collection.h>
#include <ppltasks.h>
#include <cvt/wstring>
#include <codecvt>
namespace GPIODriver
{
	
	inline char* StringtoAscIIChars(Platform::String^stringRT) {

		/*
		const std::wstring fooW(stringRT->Begin());
		const std::string fooA(fooW.begin(),  fooW.end());

		int bufferSize = WideCharToMultiByte(CP_UTF8, 0, fooW.c_str(), -1, nullptr, 0, NULL, NULL);
		auto utf8 = std::make_unique<char[]>(bufferSize);
		if (0 == WideCharToMultiByte(CP_UTF8, 0, fooW.c_str(), -1, utf8.get(), bufferSize, NULL, NULL))
			throw std::exception("Can't convert string to UTF8");
*/
		//Platform::String^ fooRT = "foo";
		stdext::cvt::wstring_convert<std::codecvt_utf8<wchar_t>> convert;
		std::string stringUtf8 = convert.to_bytes(stringRT->Data());
		const char* rawCstring = stringUtf8.c_str();

		size_t len = stringUtf8.length() + sizeof(char);
		char* charStr = new char[len];
		memcpy(charStr, stringUtf8.c_str(), len);

		/*
		size_t len = fooA.length() + sizeof(char);
		char* charStr = new char[len];
		memcpy(charStr, fooA.c_str(), len);
		*/
		return charStr;

	}
	

	inline std::string PlatFormStringtoStdString(Platform::String^stringRT) {

		//const std::wstring fooW(stringRT->Begin());
		//const std::string fooA(fooW.begin(),  fooW.end());

		stdext::cvt::wstring_convert<std::codecvt_utf8<wchar_t>> convert;
		std::string stringUtf8 = convert.to_bytes(stringRT->Data());
		//const char* rawCstring = stringUtf8.c_str();

		return stringUtf8;

	}

	inline void remove(std::wstring & str, wchar_t removechar) { // removing char from
		str.erase(std::remove(str.begin(), str.end(), removechar),
			str.end());
	}



	inline Platform::String^ StringFromAscIIChars(char* chars)
	{
		size_t newsize = strlen(chars) + 1;
		wchar_t * wcstring = new wchar_t[newsize];
		size_t convertedChars = 0;
		mbstowcs_s(&convertedChars, wcstring, newsize, chars, _TRUNCATE);
		Platform::String^ str = ref new Platform::String(wcstring);
		delete[] wcstring;
		return str;
	}
	// 

	inline std::vector<std::wstring> splitintoArray(const std::wstring& s, const std::wstring& delim)
	{
		std::vector<std::wstring> result;
		wchar_t*pbegin = (wchar_t*)s.c_str();
		wchar_t * next_token = nullptr;
		wchar_t* token;
		token = wcstok_s(pbegin, delim.c_str(), &next_token);
		while (token != nullptr) {
			// Do something with the tok
			result.push_back(token);
			token = wcstok_s(NULL, delim.c_str(), &next_token);
		}

		return result;
	}


	inline std::vector<std::string> splitintoArray(const std::string& s, const std::string& delim)
	{
		std::vector<std::string> result;
		char*pbegin = (char*)s.c_str();
		char * next_token = nullptr;
		char* token;
		token = strtok_s(pbegin, delim.c_str(), &next_token);
		while (token != nullptr) {
			// Do something with the tok
			result.push_back(token);
			token = strtok_s(NULL, delim.c_str(), &next_token);
		}

		return result;
	}

	// Time-Functions




	inline UINT64 ConvertStringToUINT64(std::wstring const& value) {
		UINT64 result = 0;
		wchar_t const* p = value.c_str();
		wchar_t const* q = p + value.size();
		while (p < q) {
			result *= 10;
			result += *(p++) - '0';
		}
		return result;
	}

	inline UINT64 getActualTimeinNanos()
	{
		SYSTEMTIME st;
		FILETIME ft;
		GetSystemTime(&st);
		SystemTimeToFileTime(&st, &ft);

		ULARGE_INTEGER uli;
		uli.LowPart = ft.dwLowDateTime; // could use memcpy here!
		uli.HighPart = ft.dwHighDateTime;

		UINT64 UnixTime = static_cast<UINT64>(uli.QuadPart);

		return UnixTime;
	}

	inline UINT64 DeltaTime(const SYSTEMTIME st1, const SYSTEMTIME st2)
	{

		union timeunion { FILETIME fileTime; ULARGE_INTEGER ul; };
		timeunion ft1;
		timeunion ft2;
		SystemTimeToFileTime(&st1, &ft1.fileTime);
		SystemTimeToFileTime(&st2, &ft2.fileTime);

		return ft2.ul.QuadPart - ft1.ul.QuadPart;
	}

	inline void NanosToLocalSystemTime(UINT64 Nanos, SYSTEMTIME *st)
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



	inline LONGLONG GetPerformanceCounter(void) // back time in µs
	{
		LARGE_INTEGER PerformanceCount;

		static LARGE_INTEGER t_start = { 0,0 };
		static LARGE_INTEGER ys_factor = { 0,0 };
		LARGE_INTEGER t;
		if (t_start.QuadPart == 0)
		{
			LARGE_INTEGER Frequency;
			// Init
			QueryPerformanceFrequency(&Frequency);
			QueryPerformanceCounter(&PerformanceCount);
			//ys_factor = 1000.0 / ((double)Frequency.QuadPart); // time in ms 


			ys_factor.QuadPart = (Frequency.QuadPart); // time in µs 

			if (ys_factor.QuadPart > 0)
			{
				t_start.QuadPart = (PerformanceCount.QuadPart * 1000000) / ys_factor.QuadPart; // in µsec
			}
			else   t_start.QuadPart = GetTickCount()*1000; // in µsec
		}

		if (ys_factor.QuadPart > 0)
		{
			QueryPerformanceCounter(&PerformanceCount);
			t.QuadPart = (PerformanceCount.QuadPart* 1000000) / ys_factor.QuadPart; // in µsec
		}
		else
		{
			t.QuadPart = GetTickCount()*1000;
		}

		return (LONGLONG)(t.QuadPart - t_start.QuadPart);
	}


	inline void HardwaitintoYsec(LONGLONG waitTime) // waiting time in  µs
	{
		LONGLONG start = GetPerformanceCounter();
		LONGLONG ende = start + waitTime;
		while (start < ende)
		{
			start = GetPerformanceCounter();
		}


	}


}