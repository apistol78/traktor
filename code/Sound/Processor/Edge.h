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

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::sound
{

class InputPin;
class OutputPin;

/*! Shader graph edge.
 * \ingroup Render
 */
class T_DLLCLASS Edge : public ISerializable
{
	T_RTTI_CLASS;

public:
	explicit Edge(const OutputPin* source = nullptr, const InputPin* destination = nullptr);

	void setSource(const OutputPin* source);

	const OutputPin* getSource() const;

	void setDestination(const InputPin* destination);

	const InputPin* getDestination() const;

	virtual void serialize(ISerializer& s) override final;

private:
	const OutputPin* m_source;
	const InputPin* m_destination;
};

}
