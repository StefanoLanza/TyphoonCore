#pragma once

#include <vector>

namespace Typhoon {

template <class Key, class T>
class vectorMap {
public:
	void               insert(Key key, T value);
	void               erase(Key key);
	std::pair<bool, T> query(Key key) const;
	void               size() const;
	void               clear();

private:
	std::vector<std::pair<Key, T>> map;
};

template <class Key, class T>
void vectorMap<Key, T>::insert(Key key, T value) {
	// Keep sorted
	auto it = std::lower_bound(map.begin(), map.end(), key, [](const auto& pair, const Key& key) { return pair.first < key; });
	assert(it == map.end() || it->first > key);
	map.insert(it, { key, std::move(value) });
}

template <class Key, class T>
void vectorMap<Key, T>::erase(Key key) {
	// Binary search
	auto it = std::lower_bound(map.begin(), map.end(), key, [](const auto& pair, const Key& key) { return pair.first < key; });
	if (it != map.end() && it->first == key) {
		map.erase(it);
	}
}

template <class Key, class T>
std::pair<bool, T> vectorMap<Key, T>::query(Key key) const {
	// Binary search
	auto it = std::lower_bound(map.begin(), map.end(), key, [](const auto& pair, const Key& key) { return pair.first < key; });
	if (it != map.end() && it->first == key) {
		return { true, it->second };
	}
	else {
		return { false, {} };
	}
}

template <class Key, class T>
void vectorMap<Key, T>::size() const {
	return map.size();
}

template <class Key, class T>
void vectorMap<Key, T>::clear() {
	map.clear();
}

} // namespace Typhoon
