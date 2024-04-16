/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Serialization/ISerializable.h"
#include "Resource/Id.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UIKIT_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class IRuntimeClass;

}

namespace traktor::uikit
{

class T_DLLCLASS Scaffolding : public ISerializable
{
	T_RTTI_CLASS;

public:
	virtual void serialize(ISerializer& s) override final;

	const resource::Id< IRuntimeClass >& getScaffoldingClass() const { return m_scaffoldingClass; }

private:
	resource::Id< IRuntimeClass > m_scaffoldingClass;
};

}
