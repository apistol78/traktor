#include "Core/Serialization/ISerializer.h"
#include "Render/IRenderSystem.h"
#include "Render/VertexElement.h"
#include "Render/VertexBuffer.h"
#include "Render/IndexBuffer.h"
#include "Render/Shader.h"
#include "Render/Shader/ShaderGraph.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "Terrain/UndergrowthEntityData.h"
#include "Terrain/Heightfield.h"
#include "Terrain/HeightfieldResource.h"
#include "Terrain/MaterialMask.h"
#include "Terrain/MaterialMaskResource.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.terrain.UndergrowthEntityData", 0, UndergrowthEntityData, world::EntityData)

UndergrowthEntity* UndergrowthEntityData::createEntity(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem) const
{
	if (!resourceManager->bind(m_heightfield) || !resourceManager->bind(m_materialMask) || !resourceManager->bind(m_shader))
		return 0;

	std::vector< render::VertexElement > vertexElements;
	vertexElements.push_back(render::VertexElement(render::DuPosition, render::DtFloat3, offsetof(UndergrowthEntity::Vertex, position)));
	vertexElements.push_back(render::VertexElement(render::DuNormal, render::DtHalf2, offsetof(UndergrowthEntity::Vertex, normal)));
	vertexElements.push_back(render::VertexElement(render::DuCustom, render::DtHalf2, offsetof(UndergrowthEntity::Vertex, texCoord)));

	Ref< render::VertexBuffer > vertexBuffer = renderSystem->createVertexBuffer(
		vertexElements,
		m_settings.density * 4 * 2 * sizeof(UndergrowthEntity::Vertex),
		true
	);
	if (!vertexBuffer)
		return 0;

	Ref< render::IndexBuffer > indexBuffer = renderSystem->createIndexBuffer(
		render::ItUInt16,
		m_settings.density * 3 * 2 * 2 * sizeof(uint16_t),
		false
	);
	if (!indexBuffer)
		return 0;

	uint16_t* index = static_cast< uint16_t* >(indexBuffer->lock());

	for (int i = 0; i < m_settings.density; ++i)
	{
		int offset = i * 4 * 2;

		*index++ = offset + 0;
		*index++ = offset + 1;
		*index++ = offset + 2;

		*index++ = offset + 0;
		*index++ = offset + 2;
		*index++ = offset + 3;

		*index++ = offset + 4;
		*index++ = offset + 5;
		*index++ = offset + 6;

		*index++ = offset + 4;
		*index++ = offset + 6;
		*index++ = offset + 7;
	}

	indexBuffer->unlock();

	render::Primitives primitives(
		render::PtTriangles,
		0,
		m_settings.density * 2 * 2,
		0,
		m_settings.density * 4 * 2 - 1
	);

	return new UndergrowthEntity(
		resourceManager,
		m_heightfield,
		m_materialMask,
		m_settings,
		vertexBuffer,
		indexBuffer,
		primitives,
		m_shader
	);
}

bool UndergrowthEntityData::serialize(ISerializer& s)
{
	if (!world::EntityData::serialize(s))
		return false;

	s >> resource::Member< Heightfield, HeightfieldResource >(L"heightfield", m_heightfield);
	s >> resource::Member< MaterialMask, MaterialMaskResource >(L"materialMask", m_materialMask);
	s >> resource::Member< render::Shader, render::ShaderGraph >(L"shader", m_shader);
	s >> Member< int32_t >(L"density", m_settings.density);
	s >> Member< float >(L"spreadDistance", m_settings.spreadDistance);
	s >> Member< float >(L"cellRadius", m_settings.cellRadius);
	s >> Member< float >(L"plantScale", m_settings.plantScale);

	return true;
}

	}
}
