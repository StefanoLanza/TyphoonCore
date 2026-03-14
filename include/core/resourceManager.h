#pragma once

#include <core/uncopyable.h>
#include <functional>
#include <memory>
#include <string_view>
#include <vector>
#ifndef _FINAL
#include <string>
#endif

namespace Typhoon {

class Logger;

struct ResourceMetaData {
#ifndef _FINAL
	std::string name;
	std::string path;
#endif
};

/*
Interface for a resource manager.
It has methods to register and delete resources, and retrieve them by name, source, type.
*/
template <class R>
class ResourceManager final : Uncopyable {
public:
	// Types
	using ResourcePtr = std::shared_ptr<R>;
	using Handle = uint32_t;
	using ResourceVector = std::vector<ResourcePtr>;
	using DeleteCallback = std::function<void(R&, const char*)>;

public:
	explicit ResourceManager(size_t capacity = 0);
	~ResourceManager();

	void SetDeleteCallback(DeleteCallback&& callback);

	Handle GetHandle(std::string_view name) const;

	ResourcePtr Find(Handle handle) const;

	ResourcePtr Find(std::string_view name) const;

	int FindIndex(Handle handle) const;

	int FindIndex(std::string_view name) const;

	bool HasResource(std::string_view name) const;

	ResourcePtr& GetResource(size_t index);

	const ResourcePtr& GetResource(size_t index) const;

	const ResourceMetaData& GetMetaData(size_t index) const;

	ResourceVector& GetResources() {
		return m_resources;
	}

	size_t GetCount() const {
		return m_resources.size();
	}

	void Delete(const ResourcePtr& resource);

	void Delete(const R* resourcePtr);

	//! Delete a resource
	void Delete(std::string_view name);

	/*/ Removes unreferenced resources (that is, resources with a ref counter == 1)
	 * This function should be called when unloading a scene
	 */
	size_t RemoveUnrefResources();

	void Register(const ResourcePtr& res, std::string_view name, std::string_view path, bool isPersistent);

private:
	void DeleteAt(size_t index);

private:
	std::vector<Handle>           m_hashTable;
	ResourceVector                m_resources;
	std::vector<ResourceMetaData> m_metaDataVec;
	std::vector<uint8_t>          m_persistent;
	DeleteCallback                m_deleteCallback;
};

} // namespace Typhoon
