/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/Ref.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MESH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class IStream;

}

namespace traktor::render
{

class IRenderSystem;
class MeshFactory;

}

namespace traktor::resource
{

class IResourceManager;

}

namespace traktor::mesh
{

class IMesh;

/*! Mesh resource.
 * \ingroup Mesh
 *
 * Base class for all mesh resources.
 */
class T_DLLCLASS MeshResource : public ISerializable
{
	T_RTTI_CLASS;

public:
	/*! Deform part; a range of the deform vertex index list dispatched with a compute technique.
	 *
	 * A part with an empty technique covers the vertices not deformed by any material; those
	 * copy their undeformed positions so the whole deform buffer is written every frame.
	 */
	struct T_DLLCLASS DeformPart
	{
		std::wstring shaderTechnique; //!< Deform compute technique of the material, empty to copy.
		uint32_t indexOffset = 0;	  //!< Offset into the deform vertex index list.
		uint32_t indexCount = 0;	  //!< Number of vertex indices.

		void serialize(ISerializer& s);
	};

	virtual Ref< IMesh > createMesh(
		const std::wstring& name,
		IStream* dataStream,
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		render::MeshFactory* meshFactory
	) const = 0;

	virtual void serialize(ISerializer& s) override;

	void setCompressed(bool compressed) { m_compressed = compressed; }

	bool isCompressed() const { return m_compressed; }

private:
	bool m_compressed = false;
};

}
