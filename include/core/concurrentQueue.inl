namespace Typhoon {

template <typename T>
ConcurrentQueue<T>::~ConcurrentQueue() {
	cancel();
	ScopedLock lock(mutex);
}

template <typename T>
bool ConcurrentQueue<T>::empty() const {
	ScopedLock lock(mutex);
	return queue.empty();
}

template <typename T>
void ConcurrentQueue<T>::push(const T& value) {
	{
		ScopedLock lock(mutex);
		queue.push(value);
	}
	condition.notify_one();
}

template <typename T>
void ConcurrentQueue<T>::push(T&& value) {
	{
		ScopedLock lock(mutex);
		queue.push(std::move(value));
	}
	condition.notify_one();
}

template <typename T>
bool ConcurrentQueue<T>::try_pop(T& result) {
	ScopedLock lock(mutex);
	if (queue.empty() || canceled) {
		return false;
	}
	result = std::move(queue.front());
	queue.pop();
	return true;
}

template <typename T>
bool ConcurrentQueue<T>::wait_pop(T& item) {
	ScopedLock lock(mutex);
	while (queue.empty() && ! canceled) {
		condition.wait(lock);
	}
	if (canceled) {
		return false;
	}
	item = std::move(queue.front());
	queue.pop();
	return true;
}

template <typename T>
void ConcurrentQueue<T>::clear() {
	ScopedLock lock(mutex);
	queue = std::move(std::queue<T> {});
}

template <typename T>
void ConcurrentQueue<T>::cancel() {
	{
		ScopedLock lock(mutex);
		canceled = true;
	}
	condition.notify_one(); // end wait_pop
}

template <typename T>
bool ConcurrentQueue<T>::isCanceled() const {
	ScopedLock lock(mutex);
	return canceled;
}

} // namespace Typhoon
