#pragma once

#include <format>

namespace Typhoon {

class Result {
public:
	explicit Result(bool result)
	    : result(result)
	    , errorMessage(nullptr) {
	}
	explicit Result(const char* errorMessage);

	bool getResult() const {
		return result;
	}
	const char* getErrorMessage() const {
		return errorMessage;
	}
	explicit operator bool() const {
		return result;
	}
	bool isOK() const {
		return result;
	}

	void setErrorMessage(std::string_view message);

	template <typename... Args>
	void setErrorMessage(const std::format_string<Args...> fmt, Args&&... args) {
		const auto [end, _] = std::format_to_n(tempBuffer, sizeof(tempBuffer) - 1, fmt, std::forward<Args>(args)...);
		*end = '\0';
		result = false;
		errorMessage = tempBuffer;
	}

private:
	bool        result;
	const char* errorMessage;
	static char tempBuffer[128];
};

} // namespace Typhoon
