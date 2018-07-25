#pragma once

#ifndef FileSystem_h
#define FileSystem_h

#include <string>

struct FileHandle
{
    char* buffer;
    size_t size;
};

namespace filesystem
{
    /**
     * Reads file from directory or zip file, depending on selected type.
     * @param _path Path to the file
     * @param _text Text flag. Set to true when reading text to add null terminator.
     * @return FileHandle structure.
     */
    extern FileHandle read_file(std::string _path, bool _text = false, bool _absolute = false);
    /**
     * Add a directory containing Assets.
     * @param _path Path of directory.
     */
    extern void add_directory(std::string _path);
    /**
     * Add file path of a Zip archive containing Assets.
     * @param _path Path of Archive.
     */
    extern void add_archive(std::string _path);
    /**
     * Get file extension from a path.
     * @param _fileName path of the file.
     * @return string File extension.
     */
	extern std::string get_file_extention(const std::string& _fileName);
    /**
     * Get the file name from a path.
     * @param _fileName path of the file.
     * @return string File name.
     */
	extern std::string get_filename(const std::string& _fileName);
    /**
     * Get the file name and extension from a path.
     * @param _fileName path of the file.
     * @return string File name with extension.
     */
	extern std::string get_file_name_and_extention(const std::string& _filePath);
    /**
     * Get the size of a given file.
     * @param _fileName path of the file.
     * @return size_t File size.
     */

	extern std::string get_file_path(const std::string& _filePath);

	extern size_t get_file_size(const std::string& _fileName);
    /**
     * Checks if a given directory exists.
     * @param _name Name of directory.
     * @return bool returns true if exists.
     */
	extern bool does_directory_exist(const std::string& _name);
    /**
     * Returns current working directory.
     * @return string Current working directory.
     */
    extern std::string get_current_working_directory();
    /**
     * Initializes a file write to the given path.
     * @param _path Path to be written to.
     * @return bool Returns true if path is valid.
     */
	extern bool write_begin(std::string _path);
    /**
     * Writes contents of buffer into the path given in WriteBegin.
     * @param _Buffer void* containing data to be written.
     * @param _Size Size of data
     * @param _Count Count of data instances to be written.
     * @param _Offset Offset from the beginning of the file.
     */
	extern void write(void* _Buffer, size_t _Size, size_t _Count, long _Offset);
    /**
     * Finishes file write by closing file. Must be called after at least WriteBegin.
     */
	extern void write_end();

	extern void copy_file(std::string input, std::string output);
    
    extern void destroy_handle(FileHandle& handle);
    
    extern bool directory_exists_internal(const std::string& path);
    extern bool create_directory(const std::string& path);
}

#endif
