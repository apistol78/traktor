/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Frustum.h"
#include "Core/Math/Matrix44.h"
#include "Render/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MESH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::mesh
{

/*!
 * \ingroup Mesh
 */
class IPartition : public Object
{
	T_RTTI_CLASS;

public:
	virtual void traverse(
		const Frustum& frustum,
		const Matrix44& worldView,
		render::handle_t worldTechnique,
		AlignedVector< uint32_t >& outPartIndices
	) const = 0;
};

}
