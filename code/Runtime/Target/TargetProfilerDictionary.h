/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/Containers/SmallMap.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RUNTIME_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::runtime
{

/*! Target profiler dictionary from running target.
 * \ingroup Runtime
 */
class T_DLLCLASS TargetProfilerDictionary : public ISerializable
{
	T_RTTI_CLASS;

public:
	TargetProfilerDictionary() = default;

	explicit TargetProfilerDictionary(const SmallMap< uint16_t, std::wstring >& dictionary);

	const SmallMap< uint16_t, std::wstring >& getDictionary() const { return m_dictionary; }

	virtual void serialize(ISerializer& s) override final;

private:
	SmallMap< uint16_t, std::wstring > m_dictionary;
};

}
