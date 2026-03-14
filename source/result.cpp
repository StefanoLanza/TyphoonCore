#include "result.h"
#include <cassert>
#include <cstring>

namespace Typhoon {

char Result::tempBuffer[128];

Result::Result(const char* errorMessage)
    : result(false) {
	assert(errorMessage != nullptr);
	setErrorMessage(errorMessage);
}

void Result::setErrorMessage(std::string_view message) {
	size_t end = std::min(sizeof(tempBuffer) - 1, message.length());
	std::memcpy(tempBuffer, message.data(), end);
	tempBuffer[end] = 0; // ensure null termination
	errorMessage = tempBuffer;
}

} // namespace Typhoon
