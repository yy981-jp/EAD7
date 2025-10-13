#pragma once


enum class INP_FROM {
	null, line, multi, file
};
Q_DECLARE_METATYPE(INP_FROM)


namespace mw {
	extern INP_FROM inp_from;
}
