/*! \file
 *  \brief Contains a graphics example
 */

#pragma once

#include "thread/thread.h"
#include "utils/random.h"
#include "syscall/guarded_graphics.h"

#include "user/graphics/cat.h"
#include "user/graphics/fire.h"
#include "user/graphics/fps.h"
#include "user/graphics/demon.h"
#include "user/graphics/pc.h"
#include "user/graphics/pong.h"
#include "user/graphics/title.h"

class GraphicsExample : public Thread {
	Demon demon;
	Cat cat;
	Fire fire;
	FPS fps;
	PC pc;
	Pong pong;
	Title title;

 public:
	GraphicsExample() {}
	void action() override;
};
