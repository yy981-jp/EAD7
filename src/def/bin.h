#pragma once
#include <vector>
#include <cstdint>
#include "secure.h"


class Bin {
	std::vector<uint8_t> data;

public:
	~Bin() {
		delm(data);
	}
};
