#pragma once

#include <core/guid.h>
#include <core/id.h>

#include <span>
#include <string_view>
#include <vector>

namespace Typhoon {

template <class Id>
class ResourceBundle {
public:
	explicit ResourceBundle(GUID guid);

	GUID                getGUID() const;
	void                addResource(Id id);
	std::span<const Id> getResources() const;

private:
	GUID            guid;
	std::vector<Id> resources;
};

template <class Id>
ResourceBundle<Id>::ResourceBundle(GUID guid)
    : guid { guid } {
}

template <class Id>
GUID ResourceBundle<Id>::getGUID() const {
	return guid;
}

template <class Id>
void ResourceBundle<Id>::addResource(Id id) {
	resources.push_back(id);
}

template <class Id>
std::span<const Id> ResourceBundle<Id>::getResources() const {
	return resources;
}

struct ResourceMetaDataNew {
	std::string_view name;
	std::string_view path;
};

} // namespace Typhoon
