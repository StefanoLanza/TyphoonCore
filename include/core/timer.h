#pragma once

#include <cstddef>
#include <cstdint>

namespace Typhoon {

class Timer final {
public:
	Timer();

	void     setFixedTimeStep(double step);
	void     setMaxTimeStep(double value);
	void     setTimeSpeed(double speed);
	double   getTimeSpeed() const;
	void     setFrameCountForAverages(size_t count);
	uint64_t getElapsedTime() const;
	uint64_t getAverageElapsedTime() const;
	double   getTotalTimeInSeconds() const;
	float    getElapsedTimeInSeconds() const;
	float    getAverageElapsedTimeInSeconds() const;
	void     update();
	void     reset();
	void     stop();
	void     start();

private:
	uint64_t fixedTimeStep;
	uint64_t maxTimeStep;
	double   speed;
	uint64_t totalTime;
	uint64_t elapsedTime;
	uint64_t elapsedTimeHistory[4];
	uint64_t prevTime;
	uint64_t baseTime;
	uint64_t lastPausedTime;
	uint64_t totalPausedTime;
	uint64_t avgElapsedTime;
	double   totalTimeInSeconds;
	float    elapsedTimeInSeconds;
	float    avgElapsedTimeInSeconds;
	size_t   historyIndex;
	size_t   historyCount;
	bool     isPaused;
};

} // namespace Typhoon
