#include "timer.h"

#ifdef WIN32
#define NOMINMAX
#include <Windows.h>
#include <algorithm>

namespace Typhoon {

namespace {

uint64_t g_ticksPerSec = 0;
double   g_ticksPerSecDouble = 0.;
double   g_secPerTickDouble = 0.;

uint64_t GetUint64Counter() {
	LARGE_INTEGER qwTime;
	QueryPerformanceCounter(&qwTime);
	return static_cast<uint64_t>(qwTime.QuadPart);
}

} // namespace

Timer::Timer()
    : fixedTimeStep(0)
    , maxTimeStep(std::numeric_limits<uint64_t>::max())
    , speed(1.f)
    , historyIndex(0)
    , historyCount(std::size(elapsedTimeHistory))
    , isPaused(false) {
	if (g_ticksPerSec == 0) {
		LARGE_INTEGER qwTicksPerSec;
		QueryPerformanceFrequency(&qwTicksPerSec);
		g_ticksPerSec = qwTicksPerSec.QuadPart;
		g_ticksPerSecDouble = static_cast<double>(g_ticksPerSec);
		g_secPerTickDouble = 1. / g_ticksPerSecDouble;
	}
	reset();
}

void Timer::setFixedTimeStep(double value) {
	fixedTimeStep = static_cast<uint64_t>(value * g_ticksPerSecDouble);
}

void Timer::setMaxTimeStep(double value) {
	maxTimeStep = static_cast<uint64_t>(value * g_ticksPerSecDouble);
}

void Timer::setTimeSpeed(double value) {
	speed = std::max(0., value);
}

double Timer::getTimeSpeed() const {
	return speed;
}

void Timer::setFrameCountForAverages(size_t count) {
	historyCount = std::max(size_t(1), std::min(count, std::size(elapsedTimeHistory)));
}

uint64_t Timer::getElapsedTime() const {
	return elapsedTime;
}

uint64_t Timer::getAverageElapsedTime() const {
	return avgElapsedTime;
}

double Timer::getTotalTimeInSeconds() const {
	return totalTimeInSeconds;
}

float Timer::getElapsedTimeInSeconds() const {
	return elapsedTimeInSeconds;
}

float Timer::getAverageElapsedTimeInSeconds() const {
	return avgElapsedTimeInSeconds;
}

void Timer::update() {
	if (fixedTimeStep == 0) {
		const uint64_t currTime = GetUint64Counter() - baseTime;
		elapsedTime = currTime - prevTime - totalPausedTime;
		prevTime = currTime;
		totalPausedTime = 0;
	}
	else {
		elapsedTime = fixedTimeStep;
	}

	elapsedTime = std::min(elapsedTime, maxTimeStep);
	elapsedTimeHistory[historyIndex % historyCount] = elapsedTime;
	++historyIndex;
	totalTime += elapsedTime;

	totalTimeInSeconds = ((double)totalTime * g_secPerTickDouble) * speed;
	elapsedTimeInSeconds = static_cast<float>(((double)elapsedTime * g_secPerTickDouble) * speed);

	const size_t count = std::min(historyCount, historyIndex);
	uint64_t     sum = 0;
	for (size_t i = 0; i < count; ++i) {
		sum += elapsedTimeHistory[i];
	}
	avgElapsedTimeInSeconds = static_cast<float>(((double)sum * g_secPerTickDouble / (double)count) * speed);
	avgElapsedTime = (uint64_t)(avgElapsedTimeInSeconds * g_ticksPerSecDouble);
}

void Timer::start() {
	if (isPaused) {
		totalPausedTime += GetUint64Counter() - lastPausedTime;
		isPaused = false;
	}
}

void Timer::stop() {
	if (! isPaused) {
		lastPausedTime = GetUint64Counter();
		isPaused = true;
	}
}

void Timer::reset() {
	totalTime = 0;
	elapsedTime = 0;
	baseTime = GetUint64Counter();
	prevTime = 0;
	lastPausedTime = 0;
	totalPausedTime = 0;
	historyIndex = 0;
	avgElapsedTime = 0;
	totalTimeInSeconds = 0.;
	elapsedTimeInSeconds = 0.;
	avgElapsedTimeInSeconds = 0.;
}

} // namespace Typhoon

#endif // WIN32