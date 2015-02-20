/*
	Copyright (C) 2003-2013 by David White <davewx7@gmail.com>
	
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef MAP_UTILS_HPP_INCLUDED
#define MAP_UTILS_HPP_INCLUDED

#include <map>

template<typename K, typename V>
const V& map_get_value_default(const std::map<K,V>& m, const K& key, const V& val) {
	typename std::map<K,V>::const_iterator i = m.find(key);
	if(i != m.end()) {
		return i->second;
	} else {
		return val;
	}
}

#endif