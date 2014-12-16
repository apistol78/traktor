#include "Core/Serialization/ISerializer.h"
#include "Heightfield/Heightfield.h"
#include "Heightfield/MaterialMask.h"
#include "Render/IRenderSystem.h"
#include "Render/VertexElement.h"
#include "Render/VertexBuffer.h"
#include "Render/IndexBuffer.h"
#include "Render/Shader.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "Terrain/UndergrowthEntityData.h"
#include "Terrain/Terrain.h"

namespace traktor
{
	namespace terrain
	{
		namespace
		{

UndergrowthEntity::Vertex packVertex(const Vector4& position, float u, float v)
{
	UndergrowthEntity::Vertex vtx;
	vtx.position[0] = position.x();
	vtx.position[1] = position.y();
	vtx.position[2] = position.z();
	vtx.texCoord[0] = floatToHalf(u);
	vtx.texCoord[1] = floatToHalf(v);
	return vtx;
}

		}

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.terrain.UndergrowthEntityData", 0, UndergrowthEntityData, world::EntityData)

UndergrowthEntity* UndergrowthEntityData::createEntity(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem) const
{
	resource::Proxy< Terrain > terrain;
	resource::Proxy< hf::MaterialMask > materialMask;
	resource::Proxy< render::Shader > shader;

	if (
		!resourceManager->bind(m_terrain, terrain) ||
		!resourceManager->bind(m_materialMask, materialMask) ||
		!resourceManager->bind(m_shader, shader)
	)
		return 0;

	std::vector< render::VertexElement > vertexElements;
	vertexElements.push_back(render::VertexElement(render::DuPosition, render::DtFloat3, offsetof(UndergrowthEntity::Vertex, position)));
	vertexElements.push_back(render::VertexElement(render::DuCustom, render::DtHalf2, offsetof(UndergrowthEntity::Vertex, texCoord)));
	T_ASSERT (render::getVertexSize(vertexElements) == sizeof(UndergrowthEntity::Vertex));

	Ref< render::VertexBuffer > vertexBuffer = renderSystem->createVertexBuffer(
		vertexElements,
		4 * 2 * sizeof(UndergrowthEntity::Vertex),
		false
	);
	if (!vertexBuffer)
		return 0;

	UndergrowthEntity::Vertex* vertex = static_cast< UndergrowthEntity::Vertex* >(vertexBuffer->lock());
	if (!vertex)
		return 0;

	Random rnd;

	const float s = m_settings.plantScale;

	Vector4 position(0.0f, 0.0f, 0.0f);
	Vector4 axisX(0.5f * s, 0.0f, 0.0f);
	Vector4 axisY(0.0f, 1.0f * s, 0.0f);
	Vector4 axisZ(0.0f, 0.0f, 0.5f * s);

	*vertex++ = packVertex(position - axisX         + axisZ, 0.0f, 1.0f);
	*vertex++ = packVertex(position - axisX + axisY + axisZ, 0.0f, 0.0f);
	*vertex++ = packVertex(position + axisX + axisY - axisZ, 0.5f, 0.0f);
	*vertex++ = packVertex(position + axisX         - axisZ, 0.5f, 1.0f);

	*vertex++ = packVertex(position - axisX         - axisZ, 0.5f, 1.0f);
	*vertex++ = packVertex(position - axisX + axisY - axisZ, 0.5f, 0.0f);
	*vertex++ = packVertex(position + axisX + axisY + axisZ, 1.0f, 0.0f);
	*vertex++ = packVertex(position + axisX         + axisZ, 1.0f, 1.0f);

	vertexBuffer->unlock();

	Ref< render::IndexBuffer > indexBuffer = renderSystem->createIndexBuffer(
		render::ItUInt16,
		3 * 2 * 2 * 2 * sizeof(uint16_t),
		false
	);
	if (!indexBuffer)
		return 0;

	uint16_t* index = static_cast< uint16_t* >(indexBuffer->lock());

	*index++ = 0;
	*index++ = 1;
	*index++ = 2;

	*index++ = 0;
	*index++ = 2;
	*index++ = 3;

	*index++ = 4;
	*index++ = 5;
	*index++ = 6;

	*index++ = 4;
	*index++ = 6;
	*index++ = 7;

	*index++ = 2;
	*index++ = 1;
	*index++ = 0;

	*index++ = 3;
	*index++ = 2;
	*index++ = 0;

	*index++ = 6;
	*index++ = 5;
	*index++ = 4;

	*index++ = 7;
	*index++ = 6;
	*index++ = 4;

	indexBuffer->unlock();

	return new UndergrowthEntity(
		terrain,
		materialMask,
		m_settings,
		vertexBuffer,
		indexBuffer,
		shader
	);
}

void UndergrowthEntityData::serialize(ISerializer& s)
{
	world::EntityData::serialize(s);

	s >> resource::Member< Terrain >(L"terrain", m_terrain);
	s >> resource::Member< hf::MaterialMask >(L"materialMask", m_materialMask);
	s >> resource::Member< render::Shader >(L"shader", m_shader);
	s >> Member< int32_t >(L"density", m_settings.density);
	s >> Member< float >(L"spreadDistance", m_settings.spreadDistance);
	s >> Member< float >(L"cellRadius", m_settings.cellRadius);
	s >> Member< float >(L"plantScale", m_settings.plantScale);
}

	}
}
