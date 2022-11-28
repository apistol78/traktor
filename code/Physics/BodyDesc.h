/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PHYSICS_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace physics
	{

class ShapeDesc;

/*! Rigid body description.
 * \ingroup Physics
 */
class T_DLLCLASS BodyDesc : public ISerializable
{
	T_RTTI_CLASS;

public:
	BodyDesc() = default;

	explicit BodyDesc(ShapeDesc* shape);

	void setShape(ShapeDesc* shape);

	Ref< const ShapeDesc > getShape() const;

	virtual void serialize(ISerializer& s) override;

private:
	Ref< ShapeDesc > m_shape;
};

	}
}

