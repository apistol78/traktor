/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/Ref.h"
#include "Core/Containers/AlignedVector.h"
#include "World/IEntityComponentData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::world
{

class EntityData;
class IEntityBuilder;

}

namespace traktor::animation
{

class JointBindingComponent;

/*!
 * \ingroup Animation
 */
class T_DLLCLASS JointBindingComponentData : public world::IEntityComponentData
{
	T_RTTI_CLASS;

public:
	struct Binding
	{
		std::wstring jointName;
		Ref< const world::EntityData > entityData;

		void serialize(ISerializer& s);
	};

	Ref< JointBindingComponent > createComponent(const world::IEntityBuilder* entityBuilder) const;

	virtual int32_t getOrdinal() const override final;

	virtual void setTransform(const world::EntityData* owner, const Transform& transform) override final;

	virtual void serialize(ISerializer& s) override final;

	const AlignedVector< Binding >& getBindings() const { return m_bindings; }

private:
	AlignedVector< Binding > m_bindings;
};

}
