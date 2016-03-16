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

#include "holo.h"
#include <stdlib.h>
#include <string.h>

struct Plugin* pluginNew(struct Config* config, const char* identifierLine) {
    struct Plugin* plugin = malloc(sizeof(struct Plugin));
    plugin->next = NULL;

    //check if the identifierLine contains an explicit executable path,
    //e.g. identifierLine = "files=./build/holo-files"
    const char* eqPos = strchr(identifierLine, '=');
    if (eqPos == NULL) {
        //`identifierLine` contains only the identifier
        plugin->identifier = strdup(identifierLine);

        //infer the executable path from the identifier
        char* relExecutablePath = stringJoin("usr/lib/holo/holo-", identifierLine);
        plugin->executablePath  = pathJoin(config->rootDir, relExecutablePath);
        free(relExecutablePath);
    } else {
        //`identifierLine` contains both identifier and executable path
        plugin->identifier     = strndup(identifierLine, eqPos - identifierLine);
        plugin->executablePath = strdup(eqPos + 1);
    }

    return plugin;
}

void pluginFree(struct Plugin* plugin) {
    //casts eliminating const are okay in this function since it is explicitly
    //used to clean up fields of `plugin` that are usually const for safety
    if (plugin) {
        free((char*) plugin->identifier);
        free((char*) plugin->executablePath);
        pluginFree(plugin->next);
        free(plugin);
    }
}
