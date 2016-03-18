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

char* pathClean(const char* path) {
    return strdup(Path(path).str().c_str());
}

char* pathJoin(const char* path1, const char* path2) {
    return strdup((Path(path1) + Path(path2)).str().c_str());
}

char* stringJoin(const char* s1, const char* s2) {
    const size_t len1 = s1 == NULL ? 0 : strlen(s1);
    const size_t len2 = s2 == NULL ? 0 : strlen(s2);
    char* s = (char*) malloc(len1 + len2 + 1); //+1 for separator and '\0'

    char* w = s; //write pointer
    strncpy(w, s1, len1); w += len1;
    strncpy(w, s2, len2); w += len2;
    *w = 0;
    return s;
}

int mkdirIncludingParents(const char* path, mode_t mode) {
    //mkdir() will return ENOENT if a parent is missing; return success and
    //all other errors immediately
    const int result = mkdir(path, mode);
    if (result == 0 || errno != ENOENT) {
        return result;
    }

    //is there a parent directory to recurse to?
    //counter-examples: path == "/" or path == "foo"
    char* p = pathClean(path);
    char* slashPos = strrchr(p, (int) '/');
    if (slashPos <= p + 1) {
        //no, there's not -> propagate the original error
        free(p);
        errno = ENOENT;
        return -1;
    }

    //truncate last path element to obtain parent path
    *slashPos = 0;
    //recurse to parent
    if (mkdirIncludingParents(p, mode) != 0) {
        const int result_errno = errno;
        free(p);
        errno = result_errno;
        return -1;
    }

    //then retry mkdir()
    return mkdir(path, mode);
}

static char* errnoToString(const char* action, const char* path, int _errno) {
    const char* error = strerror(_errno);
    const int length  = sprintf(NULL, "%s %s: %s", action, path, error);
    char* result = (char*) malloc((size_t) length + 1);
    sprintf(result, "%s %s: %s", action, path, error);
    return result;
}

char* unlinkTree(const char* path) {
    //open directory for listing
    DIR* dir = opendir(path);
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
            char* entryPath = pathJoin(path, entryName);
            char* result = errnoToString("open", entryPath, errno);
            free(entryPath);
            return result;
        }

        if (S_ISDIR(s.st_mode)) {
            //if it's a directory, recurse down
            char* entryPath = pathJoin(path, entryName);
            char* error = unlinkTree(entryPath);
            free(entryPath);
            if (error != NULL) {
                return error;
            }
        } else {
            //remove all other types of files with unlink()
            if (unlinkat(dir_fd, entryName, 0) != 0) {
                char* entryPath = pathJoin(path, entryName);
                char* result = errnoToString("remove", entryPath, errno);
                free(entryPath);
                return result;
            }
        }
    }
    free(ent);

    //close and unlink the directory
    if (closedir(dir) != 0) {
        return errnoToString("close", path, errno);
    }
    if (rmdir(path) != 0) {
        return errnoToString("remove", path, errno);
    }

    return NULL;
}
