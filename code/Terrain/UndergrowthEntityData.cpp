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

UndergrowthEntity::Vertex packVertex(const Vector4& position, float u, float v, int32_t instance)
{
	UndergrowthEntity::Vertex vtx;
	vtx.position[0] = position.x();
	vtx.position[1] = position.y();
	vtx.position[2] = position.z();
	vtx.position[3] = float(instance + 0.1f);
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
	vertexElements.push_back(render::VertexElement(render::DuPosition, render::DtFloat4, offsetof(UndergrowthEntity::Vertex, position)));
	vertexElements.push_back(render::VertexElement(render::DuCustom, render::DtHalf2, offsetof(UndergrowthEntity::Vertex, texCoord)));
	T_ASSERT (render::getVertexSize(vertexElements) == sizeof(UndergrowthEntity::Vertex));

	Ref< render::VertexBuffer > vertexBuffer = renderSystem->createVertexBuffer(
		vertexElements,
		4 * 2 * UndergrowthEntity::InstanceCount * sizeof(UndergrowthEntity::Vertex),
		false
	);
	if (!vertexBuffer)
		return 0;

	UndergrowthEntity::Vertex* vertex = static_cast< UndergrowthEntity::Vertex* >(vertexBuffer->lock());
	if (!vertex)
		return 0;

	Random rnd;

	for (int i = 0; i < UndergrowthEntity::InstanceCount; ++i)
	{
		float s = m_settings.plantScale * (rnd.nextFloat() * 0.2f + 0.8f);

		Vector4 position(0.0f, 0.0f, 0.0f);
		Vector4 axisX(0.5f * s, 0.0f, 0.0f);
		Vector4 axisY(0.0f, 1.0f * s, 0.0f);
		Vector4 axisZ(0.0f, 0.0f, 0.5f * s);

		*vertex++ = packVertex(position - axisX         + axisZ, 0.0f, 1.0f, i);
		*vertex++ = packVertex(position - axisX + axisY + axisZ, 0.0f, 0.0f, i);
		*vertex++ = packVertex(position + axisX + axisY - axisZ, 0.5f, 0.0f, i);
		*vertex++ = packVertex(position + axisX         - axisZ, 0.5f, 1.0f, i);

		*vertex++ = packVertex(position - axisX         - axisZ, 0.5f, 1.0f, i);
		*vertex++ = packVertex(position - axisX + axisY - axisZ, 0.5f, 0.0f, i);
		*vertex++ = packVertex(position + axisX + axisY + axisZ, 1.0f, 0.0f, i);
		*vertex++ = packVertex(position + axisX         + axisZ, 1.0f, 1.0f, i);
	}

	vertexBuffer->unlock();

	Ref< render::IndexBuffer > indexBuffer = renderSystem->createIndexBuffer(
		render::ItUInt16,
		3 * 2 * 2 * 2 * UndergrowthEntity::InstanceCount * sizeof(uint16_t),
		false
	);
	if (!indexBuffer)
		return 0;

	uint16_t* index = static_cast< uint16_t* >(indexBuffer->lock());

	for (int i = 0; i < UndergrowthEntity::InstanceCount; ++i)
	{
		int offset = i * 4 * 2;
		T_ASSERT (offset + 8 < 65536);

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

		*index++ = offset + 2;
		*index++ = offset + 1;
		*index++ = offset + 0;

		*index++ = offset + 3;
		*index++ = offset + 2;
		*index++ = offset + 0;

		*index++ = offset + 6;
		*index++ = offset + 5;
		*index++ = offset + 4;

		*index++ = offset + 7;
		*index++ = offset + 6;
		*index++ = offset + 4;
	}

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
