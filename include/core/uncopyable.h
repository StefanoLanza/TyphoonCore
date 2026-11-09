#pragma once

namespace Typhoon {

class Uncopyable {
public:
	Uncopyable() = default;
	Uncopyable(const Uncopyable&) = delete;
	Uncopyable& operator=(const Uncopyable&) = delete;
};

class Unmoveable {
public:
	Unmoveable() = default;
	Unmoveable(const Unmoveable&) = delete;
	Unmoveable& operator=(const Unmoveable&) = delete;
	Unmoveable(Unmoveable&&) = delete;
	Unmoveable& operator=(Unmoveable&&) = delete;
};

} // namespace Typhoon
