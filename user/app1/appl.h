#pragma once

/*! \brief Test application
 *
 *
 */
class Application {
	// Prevent copies and assignments
	Application(const Application&)            = delete;
	Application& operator=(const Application&) = delete;

 public:
	/*! \brief Constructor
	 */

	/*! \brief Contains the application code.
	 *
	 */
	void action();
};
