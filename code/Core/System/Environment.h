/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <map>
#include <string>
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! OS environment.
 * \ingroup Core
 */
class T_DLLCLASS Environment : public ISerializable
{
	T_RTTI_CLASS;

public:
	void set(const std::wstring& key, const std::wstring& value);

	void insert(const std::map< std::wstring, std::wstring >& env);

	bool has(const std::wstring& key) const;

	std::wstring get(const std::wstring& key) const;

	const std::map< std::wstring, std::wstring >& get() const;

	virtual void serialize(ISerializer& s) override final;

private:
	std::map< std::wstring, std::wstring > m_env;
};

}

