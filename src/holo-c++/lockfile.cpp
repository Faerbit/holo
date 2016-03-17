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
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

bool lockFileAcquire(struct LockFile* lock, const Config& cfg) {
    //where to store the lock file?
    if (cfg.rootDirectory == "/") {
        lock->path = strdup("/run/holo.pid");
    } else {
        lock->path = pathJoin(cfg.rootDirectory.c_str(), "holo.pid");
    }

    //acquire lock
    lock->fd = open(lock->path, O_WRONLY | O_CREAT | O_EXCL, 0644);
    if (lock->fd < 0) {
        const int open_errno = errno;
        fprintf(stderr, "Cannot create lock file %s: %s\n", lock->path, strerror(open_errno));
        if (open_errno == EEXIST) {
            fputs(
                "This usually means that another instance of Holo is currently running.\n"
                "If not, you can try to delete the lock file manually.\n",
            stderr);
        }
        return false;
    }
    //record PID in lockfile (NOTE: we don't care if this fails, it's only informative anyway)
    dprintf(lock->fd, "%d\n", getpid());
    fsync(lock->fd);

    return true;
}

void lockFileRelease(struct LockFile* lock) {
    //did we acquire the lock?
    if (lock->fd >= 0) {
        //release lock
        if (close(lock->fd) != 0) {
            fprintf(stderr, "Cannot close lock file %s: %s\n", lock->path, strerror(errno));
        }

        //cleanup file
        if (unlink(lock->path) != 0) {
            fprintf(stderr, "Cannot remove lock file %s: %s\n", lock->path, strerror(errno));
        }
    }

    free(lock->path);
}
