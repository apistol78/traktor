/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Guid.h"
#include "Core/Math/Vector4.h"
#include "Editor/Asset.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace animation
	{

/*! Animation asset.
 * \ingroup Animation
 */
class T_DLLCLASS AnimationAsset : public editor::Asset
{
	T_RTTI_CLASS;

public:
	AnimationAsset() = default;

	virtual void serialize(ISerializer& s) override final;

	const Guid& getSkeleton() const { return m_skeleton; }

	const std::wstring& getTake() const { return m_take; }

	float getScale() const { return m_scale; }

	const Vector4& getTranslate() const { return m_translate; }

	bool getRemoveLocomotion() const { return m_removeLocomotion; }

private:
	Guid m_skeleton;
	std::wstring m_take = L"Animation";
	float m_scale = 1.0f;
	Vector4 m_translate = Vector4::zero();
	bool m_removeLocomotion = true;
};

	}
}
