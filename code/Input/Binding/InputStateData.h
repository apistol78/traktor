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

class IInputNode;

/*! \brief
 * \ingroup Input
 */
class T_DLLCLASS InputStateData : public ISerializable
{
	T_RTTI_CLASS;

public:
	InputStateData();

	InputStateData(IInputNode* source);

	void setSource(IInputNode* source);

	const IInputNode* getSource() const;

	virtual void serialize(ISerializer& s) override final;

private:
	Ref< IInputNode > m_source;
};

	}
}

