/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberStl.h"
#include "Mesh/Stream/StreamMesh.h"
#include "Mesh/Stream/StreamMeshResource.h"
#include "Render/Mesh/MeshReader.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.mesh.StreamMeshResource", 3, StreamMeshResource, IMeshResource)

Ref< IMesh > StreamMeshResource::createMesh(
	const std::wstring& name,
	IStream* dataStream,
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	render::MeshFactory* meshFactory
) const
{
	Ref< StreamMesh > streamMesh = new StreamMesh();
	
	if (!resourceManager->bind(m_shader, streamMesh->m_shader))
		return 0;

	streamMesh->m_stream = dataStream;
	streamMesh->m_meshReader = new render::MeshReader(meshFactory);
	streamMesh->m_frameOffsets = m_frameOffsets;
	streamMesh->m_boundingBox = m_boundingBox;

	for (std::map< std::wstring, parts_t >::const_iterator i = m_parts.begin(); i != m_parts.end(); ++i)
	{
		render::handle_t worldTechnique = render::getParameterHandle(i->first);

		for (parts_t::const_iterator j = i->second.begin(); j != i->second.end(); ++j)
		{
			StreamMesh::Part part;
			part.shaderTechnique = render::getParameterHandle(j->shaderTechnique);
			part.meshPart = j->meshPart;
			streamMesh->m_parts[worldTechnique].push_back(part);
		}
	}

	return streamMesh;
}

void StreamMeshResource::serialize(ISerializer& s)
{
	T_ASSERT_M(s.getVersion< StreamMeshResource >() >= 3, L"Incorrect version");
	s >> resource::Member< render::Shader >(L"shader", m_shader);
	s >> MemberStlVector< uint32_t >(L"frameOffsets", m_frameOffsets);
	s >> Member< Vector4 >(L"boundingBoxMin", m_boundingBox.mn);
	s >> Member< Vector4 >(L"boundingBoxMax", m_boundingBox.mx);
	s >> MemberStlMap<
		std::wstring,
		parts_t,
		MemberStlPair<
			std::wstring,
			parts_t,
			Member< std::wstring >,
			MemberStlList< Part, MemberComposite< Part > >
		>
	>(L"parts", m_parts);
}

StreamMeshResource::Part::Part()
{
}

void StreamMeshResource::Part::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"shaderTechnique", shaderTechnique);
	s >> Member< std::wstring >(L"meshPart", meshPart);
}

	}
}
