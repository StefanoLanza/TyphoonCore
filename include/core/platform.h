#pragma once

#include <core/base.h>
#include <string>

/*!
\namespace Platform
This namespace contains platform-specific functions (for files, timers...)
*/
namespace Typhoon::Platform {

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
void queryLocalTime(LocalTime& localTime);

//! Query local time
/*! \param localTimeStr string containing the formatted local time
 */
void queryLocalTime(std::string& localTimeStr);

//! Set current directory
/*! \param directory string representing the directory
 */
void setCurrentDirectory(const char* directory);

//! Return the directory with the executable file
std::string getBinDirectory();

//! Return the default temporary directory
void getTempDirectory(char path[], size_t pathSize);

//! Return application time in seconds
double queryApplicationTime();

//! Return application ticks
int64 queryApplicationTicks();

//! Return number of ticks per second on current system
int64 getTicksPerSecond();

//! Initialize the timer
void initializeTimer();

//! Get the create and modify time of a file
bool getFileTimes(const char* filePath, FileTime* createTime, FileTime* modifyTime);

} // namespace Typhoon::Platform
