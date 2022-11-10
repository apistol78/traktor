/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Physics/Mesh.h"
#include "Physics/MeshShapeDesc.h"
#include "Resource/Member.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.physics.MeshShapeDesc", ShapeDesc::Version, MeshShapeDesc, ShapeDesc)

MeshShapeDesc::MeshShapeDesc(const resource::Id< Mesh >& mesh)
:	m_mesh(mesh)
{
}

void MeshShapeDesc::setMesh(const resource::Id< Mesh >& mesh)
{
	m_mesh = mesh;
}

const resource::Id< Mesh >& MeshShapeDesc::getMesh() const
{
	return m_mesh;
}

void MeshShapeDesc::serialize(ISerializer& s)
{
	ShapeDesc::serialize(s);
	s >> resource::Member< Mesh >(L"mesh", m_mesh);
}

	}
}
