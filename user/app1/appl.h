#pragma once

#include "thread/thread.h"

/*! \brief Test application
 *
 *
 */
class Application : public Thread {
	// Prevent copies and assignments
	Application(const Application&)            = delete;
	Application& operator=(const Application&) = delete;

 public:
	/*! \brief Constructor
	 */
	Application() {}

	/*! \brief Contains the application code.
	 *
	 */
	void action() override;
};
