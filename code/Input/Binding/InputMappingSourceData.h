/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/SmallMap.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_INPUT_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace input
	{

class IInputSourceData;

/*! Input mapping source data.
 * \ingroup Input
 *
 * Serializable description of input sources.
 */
class T_DLLCLASS InputMappingSourceData : public ISerializable
{
	T_RTTI_CLASS;

public:
	void setSourceData(const std::wstring& id, IInputSourceData* data);

	IInputSourceData* getSourceData(const std::wstring& id);

	const SmallMap< std::wstring, Ref< IInputSourceData > >& getSourceData() const;

	virtual void serialize(ISerializer& s) override final;

private:
	SmallMap< std::wstring, Ref< IInputSourceData > > m_sourceData;
};

	}
}

