namespace Typhoon {

template <typename T>
bool ConcurrentVector<T>::empty() const {
	ScopedLock lock(m_mutex);
	return m_vector.empty();
}

template <typename T>
std::size_t ConcurrentVector<T>::size() const {
	ScopedLock lock(m_mutex);
	return m_vector.size();
}

template <typename T>
void ConcurrentVector<T>::push_back(const T& value) {
	{
		ScopedLock lock(m_mutex);
		m_vector.push_back(value);
	}

	m_condition.notify_one();
}

template <typename T>
bool ConcurrentVector<T>::try_pop_back(T& result) {
	ScopedLock lock(m_mutex);

	if (m_vector.empty()) {
		return false;
	}

	result = m_vector.back();
	m_vector.pop_back();

	return true;
}

template <typename T>
T ConcurrentVector<T>::wait_pop_back() {
	ScopedLock lock(m_mutex);

	while (m_vector.empty()) {
		m_condition.wait(lock);
	}

	T result(m_vector.front());
	m_vector.pop();

	return result;
}

} // namespace Typhoon
