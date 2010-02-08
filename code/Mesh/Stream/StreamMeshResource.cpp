#include "Mesh/Stream/StreamMeshResource.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Core/Serialization/MemberComposite.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.mesh.StreamMeshResource", 0, StreamMeshResource, MeshResource)

void StreamMeshResource::setFrameOffsets(const std::vector< uint32_t >& frameOffsets)
{
	m_frameOffsets = frameOffsets;
}

const std::vector< uint32_t >& StreamMeshResource::getFrameOffsets() const
{
	return m_frameOffsets;
}

void StreamMeshResource::setBoundingBox(const Aabb& boundingBox)
{
	m_boundingBox = boundingBox;
}

const Aabb& StreamMeshResource::getBoundingBox() const
{
	return m_boundingBox;
}

void StreamMeshResource::setParts(const std::vector< Part >& parts)
{
	m_parts = parts;
}

const std::vector< StreamMeshResource::Part >& StreamMeshResource::getParts() const
{
	return m_parts;
}

bool StreamMeshResource::serialize(ISerializer& s)
{
	s >> MemberStlVector< uint32_t >(L"frameOffsets", m_frameOffsets);
	s >> Member< Vector4 >(L"boundingBoxMin", m_boundingBox.mn);
	s >> Member< Vector4 >(L"boundingBoxMax", m_boundingBox.mx);
	s >> MemberStlVector< Part, MemberComposite< Part > >(L"parts", m_parts);
	return true;
}

StreamMeshResource::Part::Part()
:	opaque(true)
{
}

bool StreamMeshResource::Part::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"name", name);
	s >> Member< Guid >(L"material", material);
	s >> Member< bool >(L"opaque", opaque);
	return true;
}

	}
}
