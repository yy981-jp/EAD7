#pragma once


enum class INP_FROM {
	null, line, multi, file
};

namespace mw {
	extern INP_FROM inp_from;
}
