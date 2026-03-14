#include <algorithm>
#include <cassert>
#include <core/hash.h>

namespace Typhoon {

template <class R>
ResourceManager<R>::ResourceManager(size_t capacity) {
	if (capacity) {
		m_hashTable.reserve(capacity);
		m_resources.reserve(capacity);
		m_metaDataVec.reserve(capacity);
	}
}

template <class R>
ResourceManager<R>::~ResourceManager() = default;

template <class R>
void ResourceManager<R>::SetDeleteCallback(DeleteCallback&& callback) {
	m_deleteCallback = std::move(callback);
}

template <class R>
typename ResourceManager<R>::Handle ResourceManager<R>::GetHandle(std::string_view name) const {
	return hash32(name.data(), name.size());
}

template <class R>
typename ResourceManager<R>::ResourcePtr ResourceManager<R>::Find(Handle handle) const {
	auto it = std::find(m_hashTable.begin(), m_hashTable.end(), handle);
	auto dist = std::distance(m_hashTable.begin(), it);
	return it == m_hashTable.end() ? nullptr : *(m_resources.begin() + dist);
}

template <class R>
typename ResourceManager<R>::ResourcePtr ResourceManager<R>::Find(std::string_view name) const {
	return Find(GetHandle(name));
}

template <class R>
int ResourceManager<R>::FindIndex(Handle handle) const {
	auto it = std::find(m_hashTable.begin(), m_hashTable.end(), handle);
	auto dist = std::distance(m_hashTable.begin(), it);
	return it == m_hashTable.end() ? -1 : static_cast<int>(dist);
}

template <class R>
int ResourceManager<R>::FindIndex(std::string_view name) const {
	return FindIndex(GetHandle(name));
}

template <class R>
bool ResourceManager<R>::HasResource(std::string_view name) const {
	return Find(name).get() != nullptr;
}

template <class R>
inline typename ResourceManager<R>::ResourcePtr& ResourceManager<R>::GetResource(size_t index) {
	return m_resources[index];
}

template <class R>
inline const typename ResourceManager<R>::ResourcePtr& ResourceManager<R>::GetResource(size_t index) const {
	return m_resources[index];
}

template <class R>
inline const ResourceMetaData& ResourceManager<R>::GetMetaData(size_t index) const {
	return m_metaDataVec[index];
}

template <class R>
void ResourceManager<R>::Delete(const ResourcePtr& resource) {
	auto it = std::find(m_resources.begin(), m_resources.end(), resource);
	assert(it != m_resources.end());

	const size_t index = std::distance(m_resources.begin(), it);
	DeleteAt(index);
}

template <class R>
void ResourceManager<R>::Delete(const R* resourcePtr) {
	auto it = std::find_if(m_resources.begin(), m_resources.end(), [=](const ResourcePtr& lhs) { return lhs.get() == resourcePtr; });
	assert(it != m_resources.end());

	const size_t index = std::distance(m_resources.begin(), it);
	DeleteAt(index);
}

template <class R>
void ResourceManager<R>::Delete(std::string_view name) {
	assert(m_hashTable.size() == m_metaDataVec.size());
	assert(m_hashTable.size() == m_resources.size());

	auto it = std::find(m_hashTable.begin(), m_hashTable.end(), GetHandle(name));
	assert(it != m_hashTable.end());

	const size_t index = std::distance(m_hashTable.begin(), it);
	DeleteAt(index);
}

template <class R>
size_t ResourceManager<R>::RemoveUnrefResources() {
	size_t numDeleted = 0;
	for (size_t index = 0; index < m_resources.size();) {
		if (! m_persistent[index]) {
			if (m_resources[index].use_count() == 1) {
				DeleteAt(index);
				++numDeleted;
			}
			else {
				++index;
			}
		}
		else {
			++index;
		}
	}
	return numDeleted;
}

template <class R>
void ResourceManager<R>::Register(const ResourcePtr& res, std::string_view name, std::string_view path, bool isPersistent) {
	const uint32_t hash = hash32(name.data(), name.length());
#ifdef _DEBUG
	if (auto it = std::find(m_hashTable.begin(), m_hashTable.end(), hash); it != m_hashTable.end()) {
		[[maybe_unused]] const auto& conflictingName = m_metaDataVec[std::distance(m_hashTable.begin(), it)].name;
		assert("Collision");
	}
#endif
	m_hashTable.push_back(hash);
	m_resources.push_back(res);
	m_persistent.push_back(isPersistent);

	// Save meta data
	ResourceMetaData metaData;
#ifndef _FINAL
	metaData.name = name;
	metaData.path = path;
#endif
	m_metaDataVec.push_back(metaData);
}

template <class R>
void ResourceManager<R>::DeleteAt(size_t index) {
	assert(index < m_metaDataVec.size());
	assert(index < m_resources.size());
	assert(index < m_hashTable.size());
	assert(index < m_persistent.size());

	// Call deleter
	if (m_deleteCallback) {
#ifndef _FINAL
		m_deleteCallback(*m_resources[index], m_metaDataVec[index].name.c_str());
#else
		m_deleteCallback(*m_resources[index], "");
#endif
	}

	// Delete meta data
	{
		auto it = m_metaDataVec.begin() + index;
		*it = m_metaDataVec.back();
		m_metaDataVec.pop_back();
	}

	// Delete resource
	{
		auto it = m_resources.begin() + index;
		*it = m_resources.back();
		m_resources.pop_back();
	}

	// Delete entry in hash table
	{
		auto it = m_hashTable.begin() + index;
		*it = m_hashTable.back();
		m_hashTable.pop_back();
	}

	// Delete entry in persistent
	{
		auto it = m_persistent.begin() + index;
		*it = m_persistent.back();
		m_persistent.pop_back();
	}
}

} // namespace Typhoon
