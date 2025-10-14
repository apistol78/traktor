/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
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

namespace traktor::animation
{

/*! Animation asset.
 * \ingroup Animation
 */
class T_DLLCLASS AnimationAsset : public editor::Asset
{
	T_RTTI_CLASS;

public:
	virtual void serialize(ISerializer& s) override final;

	const Guid& getTargetSkeleton() const { return m_targetSkeleton; }

	const std::wstring& getTake() const { return m_take; }

	void setScale(const Vector4& scale) { m_scale = scale; }

	const Vector4& getScale() const { return m_scale; }

	const Vector4& getTranslate() const { return m_translate; }

	bool getRemoveLocomotion() const { return m_removeLocomotion; }

private:
	Guid m_targetSkeleton;					//!< Target skeleton onto animation are retargeted; if no skeleton provided then assuming to be same as animation skeleton.
	std::wstring m_take = L"";
	Vector4 m_scale = Vector4::one();
	Vector4 m_translate = Vector4::zero();
	bool m_removeLocomotion = true;
};

}
