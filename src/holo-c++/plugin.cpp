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

Plugin::Plugin(const std::string& identifierLine, const Config& config) {
    //check if the identifierLine contains an explicit executable path,
    //e.g. identifierLine = "files=./build/holo-files"
    const auto eqIdx = identifierLine.find('=');
    if (eqIdx == std::string::npos) {
        //`identifierLine` contains only the identifier
        m_identifier = identifierLine;

        //infer the executable path from the identifier
        const std::string relExecutablePath = "usr/lib/holo/holo-" + identifierLine;
        m_executablePath = pathJoin(config.rootDirectory.c_str(), relExecutablePath.c_str());
    } else {
        //`identifierLine` contains both identifier and executable path
        m_identifier     = identifierLine.substr(0, eqIdx);
        m_executablePath = identifierLine.substr(eqIdx + 1);
    }
}
