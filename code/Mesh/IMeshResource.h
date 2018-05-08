/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_mesh_IMeshResource_H
#define traktor_mesh_IMeshResource_H

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

	namespace render
	{

class IRenderSystem;
class MeshFactory;

	}

	namespace resource
	{

class IResourceManager;

	}

	namespace mesh
	{

class IMesh;

/*! \brief Mesh resource.
 *
 * Base class for all mesh resources.
 */
class T_DLLCLASS IMeshResource : public ISerializable
{
	T_RTTI_CLASS;

public:
	virtual Ref< IMesh > createMesh(
		const std::wstring& name,
		IStream* dataStream,
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		render::MeshFactory* meshFactory
	) const = 0;
};

	}
}

#endif	// traktor_mesh_IMeshResource_H
