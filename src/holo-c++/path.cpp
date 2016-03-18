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

Path::Path(const std::string& path)
{
    //`m_path` is not going to be longer than `path` after cleaning (we might
    //be going to waste some bytes by not allocating optimally, but whatever)
    m_path.reserve(path.size());

    //step 1: copy from `path` to `m_path` and remove duplicate slashes
    //  e.g. "//foo/bar///" -> "/foo/bar/"
    bool sawSlash = false;
    for (char c: path) {
        const bool isSlash = c == '/';
        if (sawSlash && isSlash) {
            //duplicate slash -> don't copy
        } else {
            //acceptable character -> copy
            m_path.push_back(c);
            sawSlash = isSlash;
        }
    }

    //step 2: remove trailing slashes, except for m_path[0] if m_path == "/"
    while (m_path.size() > 1 && m_path.back() == '/') {
        m_path.pop_back();
    }
}

Path Path::operator+(const Path& rhs) const
{
    //quick path: is any of the arguments empty?
    if (m_path.size() == 0) {
        return rhs;
    }
    if (rhs.m_path.size() == 0) {
        return *this;
    }

    //if rhs is absolute, lhs does not matter
    if (rhs.m_path.front() == '/') {
        return rhs;
    }

    //join paths, but avoid duplicate slashes
    if (m_path.back() == '/') {
        return Path(m_path + rhs.m_path, WithoutCleaning{});
    } else {
        return Path(m_path + '/' + rhs.m_path, WithoutCleaning{});
    }
}
