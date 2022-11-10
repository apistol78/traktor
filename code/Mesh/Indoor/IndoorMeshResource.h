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
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Aabb3.h"
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

class T_DLLCLASS IndoorMeshResource : public MeshResource
{
	T_RTTI_CLASS;

public:
	struct T_DLLCLASS Part
	{
		std::wstring shaderTechnique;
		uint32_t meshPart = 0;

		void serialize(ISerializer& s);
	};

	typedef std::list< Part > parts_t;

	struct Sector
	{
		Vector4 min;
		Vector4 max;
		std::map< std::wstring, parts_t > parts;

		void serialize(ISerializer& s);
	};

	struct Portal
	{
		AlignedVector< Vector4 > pts;
		int32_t sectorA;
		int32_t sectorB;

		void serialize(ISerializer& s);
	};

	virtual Ref< IMesh > createMesh(
		const std::wstring& name,
		IStream* dataStream,
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		render::MeshFactory* meshFactory
	) const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	friend class IndoorMeshConverter;

	resource::Id< render::Shader > m_shader;
	AlignedVector< Sector > m_sectors;
	AlignedVector< Portal > m_portals;
};

}
