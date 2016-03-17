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

    Config cfg;
    if (!cfg.isValid) {
        return EX_CONFIG;
    }

    struct LockFile lock;
    if (!lockFileAcquire(&lock, cfg)) {
        exitStatus = EX_UNAVAILABLE;
        goto ERR_LOCK_ACQUIRE;
    }

    //iterate over plugins
    printf("DEBUG: root dir = %s\n", cfg.rootDirectory.c_str());
    for (Plugin* plugin: cfg.plugins) {
        printf("DEBUG: found plugin %s at %s\n", plugin->identifier().c_str(), plugin->executablePath().c_str());
    }

ERR_LOCK_ACQUIRE:
    lockFileRelease(&lock);

    return exitStatus;
}