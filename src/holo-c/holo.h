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

////////////////////////////////////////////////////////////////////////////////
// config.c

///The environment and plugin selection for the current run.
struct Config {
    const char*    rootDir;     ///< path to root directory (usually "/", but can differ in test runs)
    struct Plugin* firstPlugin; ///< linked list of plugins (in execution order)
};

///Initialize a Config instance for the current program run. Failures are
///reported on stderr and will result in a false return value. In this case,
///the Config instance should not be used further.
bool configInit(struct Config* config);

///Cleanup the contents of this Config instance.
void configCleanup(struct Config* config);

////////////////////////////////////////////////////////////////////////////////
// plugin.c

///Configuration for a plugin that can be run.
struct Plugin {
    const char*    identifier;
    const char*    executablePath;
    struct Plugin* next; ///< used for the linked-list structure of Config.plugins
};

///Initialize a Plugin instance for the given plugin identifier.
struct Plugin* pluginNew(struct Config* config, const char* identifier);

///Destroy a Plugin instance (and its nextPlugin recursively).
void pluginFree(struct Plugin* plugin);

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

#endif // HOLO_H
