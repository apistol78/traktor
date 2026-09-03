/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/SmallMap.h"
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

/*! Joint name translation between rig naming conventions.
 * \ingroup Animation
 *
 * Maps an animation rig's joint names onto the project skeleton's names so
 * the animation pipeline can retarget clips authored on differently named
 * rigs (e.g. generated SOMA motion onto a Mixamo skeleton). Referenced from
 * AnimationAsset; joints without an entry keep their original name.
 */
class T_DLLCLASS RigNameTranslation : public ISerializable
{
	T_RTTI_CLASS;

public:
	/*! Translate a joint name; returns the name unchanged when unmapped. */
	std::wstring translate(const std::wstring& name) const;

	const SmallMap< std::wstring, std::wstring >& getTranslations() const { return m_translations; }

	virtual void serialize(ISerializer& s) override final;

private:
	SmallMap< std::wstring, std::wstring > m_translations;
};

}
