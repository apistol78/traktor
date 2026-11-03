/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Animation/Animation.h"
#include "Core/Ref.h"
#include "Core/Serialization/ISerializable.h"
#include "Resource/Id.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Transform;

}

namespace traktor::physics
{

class PhysicsManager;

}

namespace traktor::resource
{

class IResourceManager;

}

namespace traktor::animation
{

class IPoseControllerData;
class RtStack;
class Skeleton;

/*!
 * \ingroup Animation
 */
class T_DLLCLASS RtStackData : public ISerializable
{
	T_RTTI_CLASS;

public:
	struct Layer
	{
		std::wstring name;
		resource::Id< Animation > animation;
		Ref< const IPoseControllerData > poseController;

		void serialize(ISerializer& s);
	};

	Ref< RtStack > createInstance(
		resource::IResourceManager* resourceManager,
		physics::PhysicsManager* physicsManager,
		const Skeleton* skeleton,
		const Transform& worldTransform,
		bool editor) const;

	virtual void serialize(ISerializer& s) override final;

private:
	friend class StackCompiler;

	AlignedVector< Layer > m_layers;
};

}
