/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Guid.h"
#include "Core/Containers/SmallMap.h"
#include "Mesh/MeshResource.h"
#include "Resource/Id.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MESH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class Shader;

}

namespace traktor::mesh
{

/*! Static mesh persistent data.
 * \ingroup Mesh
 */
class T_DLLCLASS StaticMeshResource : public MeshResource
{
	T_RTTI_CLASS;

public:
	struct T_DLLCLASS Part
	{
		std::wstring shaderTechnique;
		uint32_t meshPart;

		void serialize(ISerializer& s);
	};

	StaticMeshResource();

	virtual Ref< IMesh > createMesh(
		const std::wstring& name,
		IStream* dataStream,
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		render::MeshFactory* meshFactory
	) const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	friend class StaticMeshConverter;
	typedef AlignedVector< Part > parts_t;

	bool m_haveRenderMesh;
	resource::Id< render::Shader > m_shader;
	SmallMap< std::wstring, parts_t > m_parts;
};

}
