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
// config.c

///The environment and plugin selection for the current run.
struct Config {
    ///Construct a Config instance for the current program run. Failures
    ///will be reported on stderr and will result in isValid() returning
    ///false on the resulting instance.
    Config();
    ~Config();

    bool isValid;               ///< whether the constructor finished without errors
    std::string rootDirectory;  ///< path to root directory (usually "/", but can differ in test runs).
    std::string cacheDirectory; ///< path to cache directory (usually "/tmp/holo-cache").
    std::list<Plugin*> plugins; ///< list of plugins, in execution order.
};

////////////////////////////////////////////////////////////////////////////////
// lockfile.c

///LockFile represents the /run/holo.pid file.
struct LockFile {
    char* path;
    int   fd;
};

///Initialize an allocated LockFile instance by creating the lock file. Returns
///whether the lock was obtained successfully.
bool lockFileAcquire(struct LockFile* lock, const Config& cfg);

///Release the lockfile.
void lockFileRelease(struct LockFile* lock);

////////////////////////////////////////////////////////////////////////////////
// plugin.c

///Configuration for a plugin that can be run.
class Plugin {
    public:
        Plugin(const std::string& identifierLine, const Config& config);
        ~Plugin() {}

        const std::string& identifier()     const { return m_identifier; }
        const std::string& executablePath() const { return m_executablePath; }
    private:
        std::string m_identifier;
        std::string m_executablePath;
};

////////////////////////////////////////////////////////////////////////////////
// util.c

///Return a cleaned version of this path, without trailing or duplicate slashes.
char* pathClean(const char* path);

///Return a join of both paths. Both arguments must be clean, as
///defined by pathClean().
///@code
///x = pathJoin("a/b", "c/d");    // x = "a/b/c/d"
///@endcode
char* pathJoin(const char* path1, const char* path2);

///Join two strings. NULL strings are interpreted as empty strings.
char* stringJoin(const char* s1, const char* s2);

///Like mkdir(3), but create parent directories recursively like `mkdir -p`.
int mkdirIncludingParents(const char* path, mode_t mode);

///Remove the directory at `path` and all its contents. On success, return
///NULL. On failure, return an error message (which must be free'd by the
///caller).
char* unlinkTree(const char* path);

#endif // HOLO_H
