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
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int mkdirIncludingParents(const Path& path, mode_t mode) {
    //mkdir() will return ENOENT if a parent is missing; return success and
    //all other errors immediately
    const int result = mkdir(path.c_str(), mode);
    if (result == 0 || errno != ENOENT) {
        return result;
    }

    //is there a parent directory to recurse to?
    //counter-examples: path == "/" or path == "foo"
    const auto slashIdx = path.str().rfind('/');
    if (slashIdx == std::string::npos || slashIdx == 0) {
        //no, there's not -> propagate the original error
        errno = ENOENT;
        return -1;
    }

    //truncate last path element to obtain parent path
    const Path parentPath = Path(path.str().substr(0, slashIdx));
    //recurse to parent
    if (mkdirIncludingParents(parentPath, mode) != 0) {
        return -1;
    }

    //then retry mkdir()
    return mkdir(path.c_str(), mode);
}

static char* errnoToString(const char* action, const Path& path, int _errno) {
    const char* error = strerror(_errno);
    const size_t resultSize = strlen(action) + path.str().size() + strlen(error) + 4;
    char* result = (char*) malloc(resultSize);
    sprintf(result, "%s %s: %s", action, path.c_str(), error);
    return result;
}

char* unlinkTree(const Path& path) {
    //open directory for listing
    DIR* dir = opendir(path.c_str());
    if (dir == NULL) {
        return errnoToString("open", path, errno);
    }
    const int dir_fd = dirfd(dir);

    //list entries
    struct dirent* ent = (struct dirent*) malloc(offsetof(struct dirent, d_name) + NAME_MAX + 1);
    struct stat s;
    while (true) {
        struct dirent* result;
        //get next entry
        if (readdir_r(dir, ent, &result) != 0) {
            return errnoToString("read", path, errno);
        }
        //end of directory reached?
        if (result == NULL) {
            break;
        }
        const char* entryName = ent->d_name;
        //skip pseudo-entries
        if (entryName[0] == '.' && (entryName[1] == '\0' || (entryName[1] == '.' && entryName[2] == '\0'))) {
            continue;
        }

        //stat the entry
        if (fstatat(dir_fd, entryName, &s, AT_SYMLINK_NOFOLLOW) != 0) {
            return errnoToString("open", path + entryName, errno);
        }

        if (S_ISDIR(s.st_mode)) {
            //if it's a directory, recurse down
            char* error = unlinkTree(path + entryName);
            if (error != NULL) {
                return error;
            }
        } else {
            //remove all other types of files with unlink()
            if (unlinkat(dir_fd, entryName, 0) != 0) {
                return errnoToString("remove", path + entryName, errno);
            }
        }
    }
    free(ent);

    //close and unlink the directory
    if (closedir(dir) != 0) {
        return errnoToString("close", path, errno);
    }
    if (rmdir(path.c_str()) != 0) {
        return errnoToString("remove", path, errno);
    }

    return NULL;
}
