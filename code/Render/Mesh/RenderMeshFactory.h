/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Render/Mesh/MeshFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class IRenderSystem;

/*! Render mesh factory.
 * \ingroup Render
 */
class T_DLLCLASS RenderMeshFactory : public MeshFactory
{
	T_RTTI_CLASS;

public:
	explicit RenderMeshFactory(IRenderSystem* renderSystem);

	virtual Ref< Mesh > createMesh(
		const AlignedVector< VertexElement >& vertexElements,
		uint32_t vertexBufferSize,
		IndexType indexType,
		uint32_t indexBufferSize
	) const override final;

private:
	Ref< IRenderSystem > m_renderSystem;
};

}
