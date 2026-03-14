#include "guid.h"

namespace Typhoon {

namespace {
uint32_t availableGUID = 1;

}

GUID newGUID() {
	//
	return GUID { availableGUID++ };
}

} // namespace Typhoon
