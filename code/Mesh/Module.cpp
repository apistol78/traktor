/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Mesh/MeshClassFactory.h"
#	include "Mesh/Instance/InstanceMeshResource.h"
#	include "Mesh/Skinned/SkinnedMeshResource.h"
#	include "Mesh/Static/StaticMeshResource.h"

namespace traktor::mesh
{

extern "C" void __module__Traktor_Mesh()
{
	T_FORCE_LINK_REF(MeshClassFactory);
	T_FORCE_LINK_REF(InstanceMeshResource);
	T_FORCE_LINK_REF(SkinnedMeshResource);
	T_FORCE_LINK_REF(StaticMeshResource);
}

}

#endif
