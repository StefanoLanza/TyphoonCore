#pragma once

#include <mutex>
#include <thread>
#include <vector>

namespace Typhoon {

template <typename T>
class ConcurrentVector {
public:
	bool        empty() const;
	void        push_back(const T& value);
	bool        try_pop_back(T& result);
	T           wait_pop_back();
	std::size_t size() const;

private:
	typedef std::unique_lock<std::mutex> ScopedLock;
	typedef std::condition_variable      Condition;

	std::vector<T>     m_vector;
	mutable std::mutex m_mutex;
	Condition          m_condition;
};

} // namespace Typhoon

#include "concurrentVector.inl"
