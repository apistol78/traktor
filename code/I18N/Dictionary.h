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
#if defined(T_I18N_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace i18n
	{

/*! Dictionary
 * \ingroup I18N
 */
class T_DLLCLASS Dictionary : public ISerializable
{
	T_RTTI_CLASS;

public:
	bool has(const std::wstring& id) const;

	void set(const std::wstring& id, const std::wstring& text);

	void remove(const std::wstring& id);

	bool get(const std::wstring& id, std::wstring& outText) const;

	const std::map< std::wstring, std::wstring >& get() const;

	virtual void serialize(ISerializer& s) override final;

private:
	std::map< std::wstring, std::wstring > m_map;
};

	}
}

