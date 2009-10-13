/* Copyright (C) 2009 Mobile Sorcery AB

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with this program; see the file COPYING.  If not, write to the Free
Software Foundation, 59 Temple Place - Suite 330, Boston, MA
02111-1307, USA.
*/

/** \file Map.h
* \brief Thin template sorted Map.
*/

#ifndef _SE_MSAB_MAUTIL_MAP_H_
#define _SE_MSAB_MAUTIL_MAP_H_

#include <maassert.h>
#include <kazlib/dict.h>
#include "collection_common.h"

namespace MAUtil {

template<class Key, class Value>
class Map : public Dictionary<Key, Pair<Key, Value> > {
public:
	typedef Pair<Key, Value> PairKV;
protected:
	typedef Dictionary<Key, PairKV> D;
	typedef typename D::DictNode DN;
public:

	Map(int (*cf)(const Key&, const Key&) = &Compare<Key>) : D::Dictionary(cf, &PairKV::first) {}
	Pair<typename D::Iterator, bool> insert(const Key& key, const Value& value) {
		PairKV pkv = { key, value };
		return D::insert(pkv);
	}
	Value& operator[](const Key& key) {
		DN* node = (DN*)dict_lookup(&this->mDict, &key);
		if(node == NULL) {
			node = new DN();
			node->data.first = key;
			dict_insert(&this->mDict, node, &(node->data.*(this->mKeyPtr)));
		}
		return node->data.second;
	}
};

}

#endif
