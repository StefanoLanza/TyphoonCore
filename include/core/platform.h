#pragma once

#include <core/base.h>
#include <string>
#undef SetCurrentDirectory

namespace Typhoon {

/*!
\namespace Platform
This namespace contains platform-specific functions (for files, timers...)
*/
namespace Platform {

#if (defined(WIN32) || defined(WIN64))
struct FileTime {
	unsigned int v1;
	unsigned int v2;
};
#endif

struct LocalTime {
	int tm_sec;   /* seconds after the minute - [0,59] */
	int tm_min;   /* minutes after the hour - [0,59] */
	int tm_hour;  /* hours since midnight - [0,23] */
	int tm_mday;  /* day of the month - [1,31] */
	int tm_mon;   /* months since January - [0,11] */
	int tm_year;  /* years since 1900 */
	int tm_wday;  /* days since Sunday - [0,6] */
	int tm_yday;  /* days since January 1 - [0,365] */
	int tm_isdst; /* daylight savings time flag */
};

//! Query local time
/*! \param localTime output structure
 */
void QueryLocalTime(LocalTime& localTime);

//! Query local time
/*! \param localTimeStr string containing the formatted local time
 */
void QueryLocalTime(std::string& localTimeStr);

//! Set current directory
/*! \param directory string representing the directory
 */
void SetCurrentDirectory(const std::string& directory);

//! Return the directory with the executable file
std::string GetBinDirectory();

//! Return the default temporary directory
void GetTempDirectory(char path[], size_t pathSize);

//! Return application time in seconds
double QueryApplicationTime();

//! Return application ticks
int64 QueryApplicationTicks();

//! Return number of ticks per second on current system
int64 GetTicksPerSecond();

//! Initialize the timer
void InitializeTimer();

//! Get the create and modify time of a file
bool GetFileTimes(const char* filePath, FileTime* createTime, FileTime* modifyTime);

} // namespace Platform

} // namespace Typhoon
