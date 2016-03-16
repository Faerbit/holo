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

#include <sysexits.h>
#include <stdio.h>  //(f)printf

int main() {
    int exitStatus = 0;

    struct Config cfg;
    if (!configInit(&cfg)) {
        exitStatus = EX_CONFIG;
        goto ERR_CONFIG_INIT;
    }

    struct LockFile lock;
    if (!lockFileAcquire(&lock, &cfg)) {
        exitStatus = EX_UNAVAILABLE;
        goto ERR_LOCK_ACQUIRE;
    }

    //iterate over plugins
    printf("DEBUG: root dir = %s\n", cfg.rootDir);
    struct Plugin* plugin = cfg.firstPlugin;
    while (plugin) {
        printf("DEBUG: found plugin %s at %s\n", plugin->identifier, plugin->executablePath);
        plugin = plugin->next;
    }

ERR_LOCK_ACQUIRE:
    lockFileRelease(&lock);
ERR_CONFIG_INIT:
    configCleanup(&cfg);

    return exitStatus;
}
