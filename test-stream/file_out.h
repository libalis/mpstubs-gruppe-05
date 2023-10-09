/*! \file
 *  \brief \ref FileOut "File" \ref OutputStream "output" (for the voluntary C++ exercise only)
 */

#pragma once

#include "outputstream.h"

/*! \brief Write text into file
 *
 * This class allows a comfortable output to a file only by using the elementary
 * system calls `open()` / `write()` / `close()` and (optional) `fsync()`.
 * The class is derived from \ref OutputStream.
 */
class FileOut : public OutputStream {
    // TODO: Add (private) attributes, if required

 public:
	/*! \brief Constructor
	 *
	 * Opens the file for writing using the system call `open()`.
	 * \param path Path to the output file
	 *
	 *  \todo Implement constructor
	 */
	explicit FileOut(const char * path);

	/*! \brief Destructor
	 *
	 * Close the output file (using the system call `close()`)
	 *
	 *  \todo Implement destructor
	 */
	virtual ~FileOut();

	/*! \brief Get path of the output file
	 *
	 *  \return Path to output file (as defined in constructor)
	 *
	 *  \todo Implement Method
	 */
	const char * getPath();

	/*! \brief Number of output files which are currently opened (with this class)
	 *
	 *  \return Number of active files
	 *
	 *  \todo Implement Method
	 */
	static int count();

	/*! \brief Write the string to the open file.
	 *
	 * The implementation should only use the system calls `write()` and `fsync()`.
	 *
	 *  \todo Implement virtual Method
	 */
	virtual void flush() override;  //NOLINT
};
