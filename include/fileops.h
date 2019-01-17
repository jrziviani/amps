#ifndef FILEOPS_H
#define FILEOPS_H

#include "config.h"

#include <fstream>
#include <string>
#ifdef __linux__
    #include <sys/stat.h>
#endif

namespace volt
{

    bool dir_exist(const std::string &path);
    bool dir_writable(const std::string &path);
    bool dir_readable(const std::string &path);

    struct file_data
    {
        bool is_directory;
        bool is_file;
        bool is_writable;
        bool is_readable;
    };

#ifdef __linux__
    inline file_data check_file(const std::string &name)
    {
        file_data file = { false, false, false, false};
        struct stat buffer;

        if (stat(name.c_str(), &buffer) != 0) {
            return file;
        }

        file.is_readable = (buffer.st_mode & S_IRUSR) ? true : false;
        file.is_writable = (buffer.st_mode & S_IWUSR) ? true : false;
        file.is_file = ((buffer.st_mode & S_IFMT) == S_IFREG) ? true : false;
        file.is_directory = ((buffer.st_mode & S_IFMT) == S_IFREG) ? true : false;
        return file;
    }

#else
    inline file_data check_file(const std::string &name)
    {
        file_data file = { false, false, false, false};
        return file;
    }

#endif

    inline bool is_readable_directory(const std::string &path)
    {
        auto dir = check_file(path);
        if (dir.is_directory && dir.is_readable) {
            return true;
        }

        return false;
    }

    inline bool is_readable_file(const std::string &file)
    {
        auto dir = check_file(file);
        if (dir.is_file && dir.is_readable) {
            return true;
        }

        return false;
    }

    inline std::string read_full(const std::string &filename)
    {
        std::ifstream file(filename);
        if (!file.is_open()) {
            return "";
        }

        std::string content(MAX_READ_SZ + 1, '\0');
        file.read(&content[0], static_cast<std::streamsize>(MAX_READ_SZ));
        return content;
    }

    inline std::string append(const std::string &path,
                              const std::string &file)
    {
        std::string result = path;
        if (path.back() == '/') {
            result += file;
        }
        else {
            result += '/' + file;
        }

        return result;
    }
}

#endif // FILEOPS_H
