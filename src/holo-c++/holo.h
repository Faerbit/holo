/*******************************************************************************
*
* Copyright 2016 Stefan Majewsky <majewsky@gmx.net>
*
* This file is part of Holo.
*
* Holo is free software: you can redistribute it and/or modify it under the
* terms of the GNU General Public License as published by the Free Software
* Foundation, either version 3 of the License, or (at your option) any later
* version.
*
* Holo is distributed in the hope that it will be useful, but WITHOUT ANY
* WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
* A PARTICULAR PURPOSE. See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with
* Holo. If not, see <http://www.gnu.org/licenses/>.
*
*******************************************************************************/

#ifndef HOLO_H
#define HOLO_H

#include <stdbool.h>
#include <stddef.h> //size_t
#include <sys/types.h>
#include <list>
#include <string>

//forward declarations
class Plugin;

////////////////////////////////////////////////////////////////////////////////
// path.cpp

///Represents a file-system path. Paths held by instances of this class are
///always "clean", i.e. any trailing or duplicate slashes have been removed.
class Path {
    public:
        ///Construct a new instance with an empty path string.
        Path() : m_path() {}
        ///Construct a new instance by cleaning the given `path` string.
        Path(const std::string& path);
        ///@overload
        Path(const char* path) : Path(path ? std::string(path) : std::string()) {}

        ///Copy `rhs` efficiently.
        Path(const Path& rhs) : m_path(rhs.m_path) {}
        ///Move `rhs` efficiently.
        Path(Path&& rhs) : m_path(std::move(rhs.m_path)) {}

        ///Copy `rhs` efficiently.
        Path& operator=(const Path& rhs) { m_path = rhs.m_path; return *this; }
        ///Move `rhs` efficiently.
        Path& operator=(Path&& rhs) { m_path = std::move(rhs.m_path); return *this; }

        ///Concatenate paths.
        Path operator+(const Path& rhs) const;

        ///Return the path in this instance.
        operator std::string const() { return m_path; }
        ///Return the path in this instance.
        const std::string& str() const { return m_path; }
        ///Return the path in this instance. Semantics are identical to std::string::c_str().
        const char* c_str() const { return m_path.c_str(); }
    private:
        struct WithoutCleaning{};
        ///Private constructor that bypasses the cleaning of the default ctor.
        Path(const std::string& cleanPath, const WithoutCleaning&) : m_path(cleanPath) {}

        std::string m_path;
};

////////////////////////////////////////////////////////////////////////////////
// config.c

///The environment and plugin selection for the current run.
struct Config {
    ///Construct a Config instance for the current program run. Failures
    ///will be reported on stderr and will result in isValid() returning
    ///false on the resulting instance.
    Config();
    ~Config();

    bool isValid;               ///< whether the constructor finished without errors
    Path rootDirectory;         ///< usually "/", but can differ in test runs
    Path cacheDirectory;        ///< usually "/tmp/holo-cache"
    std::list<Plugin*> plugins; ///< list of plugins, in execution order.
};

////////////////////////////////////////////////////////////////////////////////
// lockfile.c

///LockFile represents the /run/holo.pid file.
class LockFile {
    public:
        ///Construct a new instance and try to acquire the lock.
        LockFile(const Config& cfg);
        ///Release the lock (if it was held).
        ~LockFile();

        ///Return if the constructor succeeded in acquiring the lock.
        bool isAcquired() const { return m_fd >= 0; }
    private:
        Path m_path;
        int  m_fd;
};

////////////////////////////////////////////////////////////////////////////////
// plugin.c

///Configuration for a plugin that can be run.
class Plugin {
    public:
        Plugin(const std::string& identifierLine, const Config& config);
        ~Plugin() {}

        const std::string& identifier() const { return m_identifier; }
        const Path& executablePath()    const { return m_executablePath; }
    private:
        std::string m_identifier;
        Path m_executablePath;
};

////////////////////////////////////////////////////////////////////////////////
// util.c

///Like mkdir(3), but create parent directories recursively like `mkdir -p`.
int mkdirIncludingParents(const Path& path, mode_t mode);

///Remove the directory at `path` and all its contents. On success, return
///NULL. On failure, return an error message (which must be free'd by the
///caller).
std::string unlinkTree(const Path& path);

#endif // HOLO_H
