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
#include <ctype.h>
#include <errno.h>
#include <stddef.h> //NULL
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

//Return a cleaned version of this path, without trailing or duplicate slashes.

static const char*  pluginCmd    = "plugin";
static const size_t pluginCmdLen = 6; // == strlen(pluginCmd)

bool configInit(struct Config* cfg) {
    bool success = true; //unless set to false

    //get the root directory
    char* const rootDir = getenv("HOLO_ROOT_DIR");
    if (rootDir == NULL || *rootDir == 0) {
        cfg->rootDir = strdup("/");
    } else {
        cfg->rootDir = pathClean(rootDir);
    }

    //start with empty plugin list
    cfg->firstPlugin = NULL;
    struct Plugin* lastPlugin = NULL;

    //open $root/etc/holorc
    char* const rcPath = pathJoin(cfg->rootDir, "etc/holorc");
    FILE* file = fopen(rcPath, "r");
    if (file == NULL) {
        fprintf(stderr, "open %s: %s\n", rcPath, strerror(errno));
        success = false;
        goto ERR_FOPEN;
    }

    //read line by line
    size_t bufferLength = 1000;
    char*  buffer       = malloc(bufferLength + 1);
    while (true) {
        const ssize_t readLength = getline(&buffer, &bufferLength, file);
        if (readLength < 0) {
            break; //EOF
        }

        //trim leading spaces
        char* line = buffer;
        size_t len = readLength;
        while (len > 0 && isspace(*line)) {
            line++; len--;
        }
        //trim trailing spaces
        char* last = line + len - 1;
        while (len > 0 && isspace(*last)) {
            *last-- = 0;
            len--;
        }
        //skip empty lines and comments
        if (len == 0 || *line == '#') {
            continue;
        }

        //recognize commands
        if (strncmp(line, pluginCmd, pluginCmdLen) == 0) {
            line += pluginCmdLen;
            len  -= pluginCmdLen;
            //trim spaces before argument
            while (len > 0 && isspace(*line)) {
                line++; len--;
            }

            //`line` now contains the plugin identifier
            struct Plugin* p = pluginNew(cfg, line);
            //append plugin to list
            if (lastPlugin) {
                lastPlugin->next = p;
            } else {
                cfg->firstPlugin = p;
            }
            lastPlugin = p;
        } else {
            fprintf(stderr, "read %s: unrecognized line: %s\n", rcPath, line);
            success = false; //...but keep going to report all broken lines
        }
    } //read next line

    free(buffer);
    fclose(file);
ERR_FOPEN:
    free(rcPath);

    return success;
}

void configCleanup(struct Config* cfg) {
    //casts eliminating const are okay in this function since it is explicitly
    //used to clean up fields of `cfg` that are usually const for safety
    free((char*) cfg->rootDir);

    pluginFree(cfg->firstPlugin);
}
