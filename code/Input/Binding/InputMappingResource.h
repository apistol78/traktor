/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

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

class InputMappingSourceData;
class InputMappingStateData;

/*! \brief
 * \ingroup Input
 */
class T_DLLCLASS InputMappingResource : public ISerializable
{
	T_RTTI_CLASS;

public:
	InputMappingResource();

	InputMappingResource(InputMappingSourceData* sourceData, InputMappingStateData* stateData);

	InputMappingSourceData* getSourceData() const { return m_sourceData; }

	InputMappingStateData* getStateData() const { return m_stateData; }

	virtual void serialize(ISerializer& s) override final;

private:
	Ref< InputMappingSourceData > m_sourceData;
	Ref< InputMappingStateData > m_stateData;
};

	}
}

