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
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class InputPin;
class OutputPin;

/*! Graph edge.
 * \ingroup Render
 */
class T_DLLCLASS Edge : public ISerializable
{
	T_RTTI_CLASS;

public:
	Edge() = default;

	explicit Edge(const OutputPin* source, const InputPin* destination);

	const OutputPin* getSource() const { return m_source; }

	const InputPin* getDestination() const { return m_destination; }

	virtual void serialize(ISerializer& s) override final;

private:
	const OutputPin* m_source = nullptr;
	const InputPin* m_destination = nullptr;
};

	}
}

