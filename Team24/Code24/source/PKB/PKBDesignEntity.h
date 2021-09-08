#pragma once
#include <type_traits>

enum class PKBDesignEntity {
	Read = 0,
	Print = 1,
	Assign = 2,
	Call = 3,
	While = 4,
	If = 5,
	Procedure = 6,
	_ = 7
};



// generic iterator over enums
template < typename C, C beginVal, C endVal>
class Iterator {
	typedef typename std::underlying_type<C>::type val_t;
	int val;
public:
	Iterator(const C& f) : val(static_cast<val_t>(f)) {}
	Iterator() : val(static_cast<val_t>(beginVal)) {}
	Iterator operator++() {
		++val;
		return *this;
	}
	C operator*() { return static_cast<C>(val); }
	Iterator begin() { return *this; } //default ctor is good
	Iterator end() {
		static const Iterator endIter = ++Iterator(endVal); // cache it
		return endIter;
	}
	bool operator!=(const Iterator& i) { return val != i.val; }
};

typedef Iterator<PKBDesignEntity, PKBDesignEntity::Read, PKBDesignEntity::_> PKBDesignEntityIterator;