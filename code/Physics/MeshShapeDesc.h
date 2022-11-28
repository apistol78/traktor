/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Physics/ShapeDesc.h"
#include "Resource/Id.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PHYSICS_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::physics
{

class Mesh;

/*! Mesh collision shape description.
 * \ingroup Physics
 */
class T_DLLCLASS MeshShapeDesc : public ShapeDesc
{
	T_RTTI_CLASS;

public:
	MeshShapeDesc() = default;

	explicit MeshShapeDesc(const resource::Id< Mesh >& mesh);

	void setMesh(const resource::Id< Mesh >& mesh);

	const resource::Id< Mesh >& getMesh() const;

	virtual void serialize(ISerializer& s) override final;

private:
	resource::Id< Mesh > m_mesh;
};

}
