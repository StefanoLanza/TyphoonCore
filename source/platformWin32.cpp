#if (defined(WIN32) || defined(WIN64))

#include "platform.h"
#include <cassert>
#include <ctime>
#include <filesystem>

#define NOMINMAX
#include <windows.h>
#undef setCurrentDirectory

#include <minwindef.h>
#include <sys/utime.h>

namespace Typhoon::Platform {

namespace {
LONGLONG ticksPerSec = 0;
BOOL     usingQPF = false;
bool     timerInit = false;
} // namespace

void queryLocalTime(LocalTime& localTime) {
	time_t    long_time = time(NULL); // * Get time as long integer.
	struct tm winLocalTime;
	localtime_s(&winLocalTime, &long_time); // * Convert to local time.

	localTime.tm_sec = winLocalTime.tm_sec;
	localTime.tm_min = winLocalTime.tm_min;
	localTime.tm_hour = winLocalTime.tm_hour;
	localTime.tm_mday = winLocalTime.tm_mday;
	localTime.tm_mon = winLocalTime.tm_mon;
	localTime.tm_year = winLocalTime.tm_year;
	localTime.tm_wday = winLocalTime.tm_wday;
	localTime.tm_yday = winLocalTime.tm_yday;
	localTime.tm_isdst = winLocalTime.tm_isdst;
}

void queryLocalTime(std::string& localTimeStr) {
	time_t    long_time = time(NULL); // * Get time as long integer.
	struct tm winLocalTime;
#if _MSC_VER > 1000
	localtime_s(&winLocalTime, &long_time);
#else
	winLocalTime = *localtime(&long_time); // * Convert to local time.
#endif
	char str[64];
	asctime_s(str, 64, &winLocalTime);
	localTimeStr = std::string(str);
}

void setCurrentDirectory(const char* directory) {
	assert(directory);
	SetCurrentDirectoryA(directory);
}

std::string getBinDirectory() {
	wchar_t filepath[MAX_PATH];
	GetModuleFileNameW(NULL, filepath, MAX_PATH);
	std::filesystem::path path(filepath);
	return path.remove_filename().generic_string();
}

void getTempDirectory(char path[], size_t pathSize) {
	GetTempPathA(static_cast<DWORD>(pathSize), path);
}

void initializeTimer() {
	// Use QueryPerformanceFrequency() to get frequency of timer.  If QPF is
	// not supported, we will timeGetTime() which returns milliseconds
	LARGE_INTEGER qwTicksPerSec {};
	usingQPF = QueryPerformanceFrequency(&qwTicksPerSec);
	if (usingQPF) {
		ticksPerSec = qwTicksPerSec.QuadPart;
	}
	timerInit = true;
}

double queryApplicationTime() {
	if (! timerInit) {
		initializeTimer();
	}
	double currAppTime = 0;
	if (usingQPF) {
		LARGE_INTEGER qwTime;
		if (QueryPerformanceCounter(&qwTime)) {
			currAppTime = (double)(qwTime.QuadPart) / (double)ticksPerSec;
		}
	}
	else {
		// Get the time using timeGetTime()
		DWORD appTime = timeGetTime();
		currAppTime = (double)(appTime) * 0.001;
	}

	return currAppTime;
}

int64 queryApplicationTicks() {
	if (! timerInit) {
		initializeTimer();
	}
	if (usingQPF) {
		LARGE_INTEGER qwTime;
		QueryPerformanceCounter(&qwTime);
		return static_cast<int64>(qwTime.QuadPart);
	}

	return 0;
}

int64 getTicksPerSecond() {
	if (! timerInit) {
		initializeTimer();
	}
	return static_cast<int64>(ticksPerSec);
}

bool getFileTimes(const char* filePath, FileTime* createTime, FileTime* modifyTime) {
	WIN32_FIND_DATA findData;
#ifdef UNICODE
	UTF16 fp[512];
	convertUTF8toUTF16((UTF8*)filePath, fp, sizeof(fp));
#else
	const char* fp = filePath;
#endif

	HANDLE h = FindFirstFile(fp, &findData);
	if (h == INVALID_HANDLE_VALUE)
		return false;

	if (createTime) {
		createTime->v1 = findData.ftCreationTime.dwLowDateTime;
		createTime->v2 = findData.ftCreationTime.dwHighDateTime;
	}
	if (modifyTime) {
		modifyTime->v1 = findData.ftLastWriteTime.dwLowDateTime;
		modifyTime->v2 = findData.ftLastWriteTime.dwHighDateTime;
	}
	FindClose(h);
	return true;
}

} // namespace Typhoon::Platform

#endif
