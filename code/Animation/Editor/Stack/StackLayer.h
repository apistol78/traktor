/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Core/Ref.h"
#include "Core/Serialization/ISerializable.h"
#include "Resource/Id.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::animation
{

class Animation;
class IPoseControllerData;
class ITransformTimeData;

/*! Animation stack layer.
 * \ingroup Animation
 */
class T_DLLCLASS StackLayer : public ISerializable
{
	T_RTTI_CLASS;

public:
	StackLayer() = default;

	explicit StackLayer(const std::wstring& name, const resource::Id< Animation >& animation);

	virtual void serialize(ISerializer& s) override;

	const std::wstring& getName() const { return m_name; }

	const resource::Id< Animation >& getAnimation() const { return m_animation; }

	const IPoseControllerData* getPoseController() const { return m_poseController; }

	const ITransformTimeData* getTransformTime() const { return m_transformTime; }

private:
	std::wstring m_name;
	resource::Id< Animation > m_animation;
	Ref< const IPoseControllerData > m_poseController;
	Ref< const ITransformTimeData > m_transformTime;
};

}
