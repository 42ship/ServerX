#pragma once

#include <string>
#include <map>
#include <iostream>
#include <fcntl.h>           /* Definition of AT_* constants */
#include <sys/stat.h>

#define MIME_TYPES_PATH "config/mime.types"

class MimeTypes
{
public:

	/**
	 * @brief Constructs the MimeTypes object and loads MIME types from the specified file.
	 *
	 * This constructor initializes the MIME types map by reading from the provided
	 * file path. It sets the last reload time to the last modification time of the file.
	 * If the file cannot be loaded, throws a runtime_error exception.
	 *
	 * @param path Optional parameter path to the MIME types configuration file. Defaults to "MIME_TYPES_PATH".
	 */
	MimeTypes(const std::string& path = MIME_TYPES_PATH);

	/**
	 * @brief Copy constructor.
	 *
	 * This constructor creates a new MimeTypes object as a copy of an existing one.
	 *
	 * @param other The MimeTypes object to copy.
	 */
	MimeTypes(const MimeTypes& other);

	/**
	 * @brief Assignment operator.
	 *
	 * This operator assigns the contents of one MimeTypes object to another.
	 *
	 * @param other The MimeTypes object to assign from.
	 * @return A reference to this MimeTypes object.
	 */
	MimeTypes& operator=(const MimeTypes& other);
	
	/**
	 * @brief Retrieves the MIME type associated with a given file extension.
	 *
	 * This method looks up the MIME type corresponding to the provided file
	 * extension in the loaded MIME types map..
	 *
	 * @param extension The file extension (without the leading dot) for which
	 *                  to retrieve the MIME type.
	 * @return The corresponding MIME type as a string, or "text/plain"
	 *         if the extension is not recognized.
	 */
	const std::string getMimeType(const std::string &extension);

	~MimeTypes();

private:
	/*map<extensoin, type> mimeTypes_*/
	std::map<std::string, std::string> mimeTypes_; // Map of file extensions to MIME types
	std::string filePath_; // Path to the MIME types configuration file
	struct timespec rtime_; // last reload time

	/**
	 * @brief checks if the MIME types file has been modified since the last load.
	 */
	bool wasChanged();

	/**
	 * @brief Reloads the MIME types from the configuration file.
	 *
	 * This method reads the MIME types file specified by `filePath_` and
	 * populates the `mimeTypes_` map with the latest MIME type definitions.
	 * It updates the `rtime_` to reflect the time of the last successful
	 * reload.
	 */
	void reload();

	size_t findFirstSpace(const std::string str);
	size_t findFirstNonSpace(const std::string str, size_t startPos);
};

