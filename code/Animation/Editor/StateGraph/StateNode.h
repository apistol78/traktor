/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Animation/Pose.h"
#include "Core/Serialization/ISerializable.h"

#include <string>

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::animation
{

/*! Base state node.
 * \ingroup Animation
 */
class T_DLLCLASS StateNode : public ISerializable
{
	T_RTTI_CLASS;

public:
	StateNode() = default;

	explicit StateNode(const std::wstring& name);

	const std::wstring& getName() const;

	void setPosition(const std::pair< int, int >& position);

	const std::pair< int, int >& getPosition() const;

	virtual void serialize(ISerializer& s) override;

private:
	std::wstring m_name;
	std::pair< int, int > m_position = { 0, 0 };
};

}
