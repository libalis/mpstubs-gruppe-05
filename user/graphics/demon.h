/*! \file
 *  \brief Beware, demons ahead
 */

#pragma once

#include "utils/png.h"

class Demon {
	PNG image;

 public:
	explicit Demon(const char * image = "demon.png");
	void summon();
};
