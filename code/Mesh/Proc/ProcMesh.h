/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Mesh/IMesh.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MESH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::mesh
{

/*! Procedural mesh.
 *
 * A procedural mesh is not directly
 * renderable but are primarily useful
 * for other systems which need to
 * amplify/modify meshes before rendering.
 */
class T_DLLCLASS ProcMesh : public IMesh
{
	T_RTTI_CLASS;

public:

private:
	friend class ProcMeshResource;
};

}
