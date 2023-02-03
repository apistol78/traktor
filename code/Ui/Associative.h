/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::ui
{

/*! Associate container.
 * \ingroup UI
 *
 * Each UI widget is derived from this container to make it
 * possible to attach objects by names to widgets or widget items.
 */
class T_DLLCLASS Associative
{
public:
	virtual ~Associative();

	/*! Remove all objects. */
	void removeAllData();

	/*! Set named object. */
	void setData(const std::wstring& key, Object* data);

	/*! Get named object. */
	Object* getData(const std::wstring& key) const;

	/*! Get named object, ensure proper type. */
	template < typename DataType >
	DataType* getData(const std::wstring& key) const
	{
		Object* data = getData(key);
		return dynamic_type_cast< DataType* >(data);
	}

	/*! Copy all data from another object. */
	void copyData(const Associative* source);

private:
	class Impl;

	Impl* m_impl = nullptr;
};

}
