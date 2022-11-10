/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Mesh/Proc/ProcMesh.h"
#include "Mesh/Proc/ProcMeshResource.h"

namespace traktor::mesh
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.mesh.ProcMeshResource", 0, ProcMeshResource, MeshResource)

Ref< IMesh > ProcMeshResource::createMesh(
	const std::wstring& name,
	IStream* dataStream,
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	render::MeshFactory* meshFactory
) const
{
	Ref< ProcMesh > procMesh = new ProcMesh();

	return procMesh;
}

void ProcMeshResource::serialize(ISerializer& s)
{
	MeshResource::serialize(s);
}

}
