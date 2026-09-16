#pragma once

#include <cstdint>

namespace Typhoon {

struct Handle {
	static constexpr uint32_t indexBits = 24;
	static constexpr uint32_t generationBits = 8;
	static constexpr uint32_t invalidIndex = (1u << indexBits) - 1u;

	uint32_t index : indexBits;
	uint32_t generation : generationBits;

	constexpr Handle()
	    : index(0)
	    , generation(0) {
	}

	constexpr Handle(uint32_t index_, uint32_t generation_)
	    : index(index_)
	    , generation(generation_) {
	}

	explicit constexpr Handle(uint32_t value)
	    : index(value >> generationBits)
	    , generation(value & 0xffu) {
	}

	constexpr void set(uint32_t index_, uint32_t generation_) {
		index = index_;
		generation = generation_;
	}

	constexpr void set(uint32_t value) {
		index = value >> generationBits;
		generation = value & 0xffu;
	}

	constexpr uint32_t getIndex() const {
		return index;
	}

	constexpr uint32_t getGeneration() const {
		return generation;
	}

	constexpr uint32_t get() const {
		return (index << generationBits) | generation;
	}

	constexpr bool isNull() const {
		return index == 0 && generation == 0;
	}

	constexpr bool isValid() const {
		return ! isNull();
	}

	constexpr explicit operator bool() const {
		return ! isNull();
	}

	constexpr void reset() {
		index = 0;
		generation = 0;
	}
};

static constexpr Handle nullHandle {};

constexpr bool operator==(Handle a, Handle b) {
	return a.get() == b.get();
}

constexpr bool operator!=(Handle a, Handle b) {
	return ! (a == b);
}

} // namespace Typhoon
