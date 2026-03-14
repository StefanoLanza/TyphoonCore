#pragma once

#include <cstdint>

namespace Typhoon {

struct Handle {
	using Storage = uint32_t;
	uint32_t index : 24;
	uint32_t generation : 8;

	constexpr Handle()
	    : index(0)
	    , generation(0) {
	}
	Handle(uint32_t index, uint32_t generation)
	    : index(index)
	    , generation(generation) {
	}
	explicit Handle(uint32_t value) {
		set(value);
	}
	void set(uint32_t index_, uint32_t generation_) {
		index = index_;
		generation = generation_;
	}
	void set(uint32_t value) {
		index = value >> 8;
		generation = value & 0xFF;
	}
	uint32_t getIndex() const {
		return index;
	}
	uint32_t getGeneration() const {
		return generation;
	}
	uint32_t get() const {
		return (index << 8) | (generation);
	}
	explicit operator bool() const {
		return get() != 0;
	}
	void reset() {
		index = 0;
		generation = 0;
	}
	bool isValid() const {
		return get() != 0;
	}
	bool isNull() const {
		return get() == 0;
	}
};

static constexpr Handle nullHandle;

inline bool operator==(const Handle& a, const Handle& b) {
	return a.index == b.index && a.generation == b.generation;
}
inline bool operator!=(const Handle& a, const Handle& b) {
	return ! (a == b);
}

} // namespace Typhoon
