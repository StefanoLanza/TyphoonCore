#include "flags.h"

#if 0

void test() {
	enum class Actions : uint8_t {
		move = 0x1,
		run = 0x2,
		jump = 0x4,
	};

	enum class Colors : uint8_t {
		red = 0x1,
		green = 0x2,
	};

	using ActionFlags = Typhoon::Flags<Actions>;
	using ColorFlags = Typhoon::Flags<Colors>;

	ActionFlags actionFlags { Actions::jump, Actions::move };
	ActionFlags actionFlags2 { Actions::jump };
	ColorFlags  colorFlags { Colors::red };

	actionFlags.set(Actions::move);
	actionFlags.unset(Actions::jump);
	[[maybe_unused]] bool jumping = actionFlags.isSet(Actions::jump);
	actionFlags.flip(Actions::jump);
	// actionFlags.set(WrongEnum::red);
	ActionFlags f = actionFlags | actionFlags2;
	f = actionFlags & actionFlags2;
	f = actionFlags ^ actionFlags2;
	f = actionFlags | Actions::run;
	actionFlags |= Actions::run;
}
#endif