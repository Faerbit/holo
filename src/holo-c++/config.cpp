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
#include <unistd.h>

static const char*  pluginCmd    = "plugin";
static const size_t pluginCmdLen = 6; // == strlen(pluginCmd)

//Step 1 of Config::Config(): Find root directory and setup fixed subdirectories.
static bool prepareRootDir(Config& cfg) {
    //get the root directory
    cfg.rootDirectory = getenv("HOLO_ROOT_DIR");
    if (cfg.rootDirectory.empty()) {
        cfg.rootDirectory  = "/";
        cfg.cacheDirectory = "/tmp/holo-cache";
    } else {
        cfg.rootDirectory  = pathClean(cfg.rootDirectory.c_str());
        cfg.cacheDirectory = pathJoin(cfg.rootDirectory.c_str(), "tmp/holo-cache");
    }

    //if the cache directory exists from a previous run, remove it recursively
    char* error;
    if (unlink(cfg.cacheDirectory.c_str()) != 0) {
        switch (errno) {
        case EISDIR:
            //is a directory -> remove recursively
            error = unlinkTree(cfg.cacheDirectory.c_str());
            if (error != NULL) {
                fprintf(stderr, "Cannot remove %s: %s\n", cfg.cacheDirectory.c_str(), error);
                free(error);
                return false;
            }
            break;
        case ENOENT:
            //does not exist -> nothing to do
            break;
        default:
            //unexpected error
            fprintf(stderr, "Cannot remove %s: %s\n", cfg.cacheDirectory.c_str(), strerror(errno));
            break;
        }
    }

    //initialize the cache directory
    if (mkdirIncludingParents(cfg.cacheDirectory.c_str(), 0755) != 0) {
        fprintf(stderr, "Cannot create %s: %s\n", cfg.cacheDirectory.c_str(), strerror(errno));
        return false;
    }
    return true;
}

//Step 2 of Config::Config(): Read /etc/holorc and validate plugin setup.
//Requires that step 1 (prepareRootDir) is already done.
static bool readConfig(Config& cfg) {
    bool success = true; //unless set to false
    size_t bufferLength; char* buffer; //declared in advance to avoid goto crossing initialization

    //open $root/etc/holorc
    char* const rcPath = pathJoin(cfg.rootDirectory.c_str(), "etc/holorc");
    FILE* file = fopen(rcPath, "r");
    if (file == NULL) {
        fprintf(stderr, "open %s: %s\n", rcPath, strerror(errno));
        success = false;
        goto ERR_FOPEN;
    }

    //read line by line
    bufferLength = 1000;
    buffer       = (char*) malloc(bufferLength + 1);
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
            cfg.plugins.push_back(new Plugin(line, cfg));
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

Config::Config() {
    isValid = prepareRootDir(*this) && readConfig(*this);
}

Config::~Config() {
    //cleanup runtime cache
    char* error = unlinkTree(cacheDirectory.c_str());
    if (error != NULL) {
        fprintf(stderr, "Cannot remove %s: %s\n", cacheDirectory.c_str(), error);
        free(error);
        //...but keep going
    }

    plugins.clear();
}
