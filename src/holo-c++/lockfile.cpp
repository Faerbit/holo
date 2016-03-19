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

LockFile::LockFile(const Config& cfg)
    : m_path(cfg.rootDirectory.str() == "/" ? "/run/holo.pid" : (cfg.rootDirectory + "holo.pid"))
    , m_fd(-1)
{
    //acquire lock
    m_fd = open(m_path.c_str(), O_WRONLY | O_CREAT | O_EXCL, 0644);
    if (m_fd < 0) {
        const int open_errno = errno;
        fprintf(stderr, "Cannot create lock file %s: %s\n", m_path.c_str(), strerror(open_errno));
        if (open_errno == EEXIST) {
            fputs(
                "This usually means that another instance of Holo is currently running.\n"
                "If not, you can try to delete the lock file manually.\n",
            stderr);
        }
    }
    //record PID in lockfile (NOTE: we don't care if this fails, it's only informative anyway)
    dprintf(m_fd, "%d\n", getpid());
    fsync(m_fd);
}

LockFile::~LockFile() {
    //did we acquire the lock?
    if (isAcquired()) {
        //release lock
        if (close(m_fd) != 0) {
            fprintf(stderr, "Cannot close lock file %s: %s\n", m_path.c_str(), strerror(errno));
        }

        //cleanup file
        if (unlink(m_path.c_str()) != 0) {
            fprintf(stderr, "Cannot remove lock file %s: %s\n", m_path.c_str(), strerror(errno));
        }
    }
}
