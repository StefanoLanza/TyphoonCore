#pragma once

#include <mutex>
#include <queue>

namespace Typhoon {

template <typename T>
class ConcurrentQueue {
public:
	~ConcurrentQueue();
	bool empty() const;
	void push(const T& value);
	void push(T&& value);
	bool try_pop(T& item);
	bool wait_pop(T& item);
	void clear();
	void cancel();
	bool isCanceled() const;

private:
	using ScopedLock = std::unique_lock<std::mutex>;
	using Condition = std::condition_variable;

	std::queue<T>      queue;
	mutable std::mutex mutex;
	Condition          condition;
	bool               canceled = false;
};

} // namespace Typhoon

#include "concurrentQueue.inl"
