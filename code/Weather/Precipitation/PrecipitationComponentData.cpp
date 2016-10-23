#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/ISerializer.h"
#include "Render/IndexBuffer.h"
#include "Render/IRenderSystem.h"
#include "Render/Shader.h"
#include "Render/VertexBuffer.h"
#include "Render/VertexElement.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "Weather/Precipitation/PrecipitationComponent.h"
#include "Weather/Precipitation/PrecipitationComponentData.h"

namespace traktor
{
	namespace weather
	{
		namespace
		{

#pragma pack(1)
struct Vertex
{
	float edge;
	float layer;
	float texCoord[2];
};
#pragma pack()

const Vertex c_template[] =
{
	{ 0.0f, 0.0f, { 0.0f, 1.0f } },
	{ 1.0f, 0.0f, { 0.0f, 0.0f } },
	{ 2.0f, 0.0f, { 1.0f, 0.0f } },
	{ 3.0f, 0.0f, { 1.0f, 1.0f } }
};

		}

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.weather.PrecipitationComponentData", 0, PrecipitationComponentData, world::IEntityComponentData)

PrecipitationComponentData::PrecipitationComponentData()
:	m_layers(1)
,	m_distance(1.0f)
{
}

Ref< PrecipitationComponent > PrecipitationComponentData::createComponent(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem) const
{
	resource::Proxy< render::Shader > shader;
	if (!resourceManager->bind(m_shader, shader))
		return 0;

	std::vector< render::VertexElement > vertexElements;
	vertexElements.push_back(render::VertexElement(render::DuCustom, render::DtFloat1, offsetof(Vertex, edge), 0));
	vertexElements.push_back(render::VertexElement(render::DuCustom, render::DtFloat1, offsetof(Vertex, layer), 1));
	vertexElements.push_back(render::VertexElement(render::DuCustom, render::DtFloat2, offsetof(Vertex, texCoord), 2));

	uint32_t vertexSize = render::getVertexSize(vertexElements);
	T_FATAL_ASSERT (vertexSize == sizeof(Vertex));

	Ref< render::VertexBuffer > vertexBuffer = renderSystem->createVertexBuffer(
		vertexElements,
		m_layers * sizeof_array(c_template) * vertexSize,
		false
	);
	if (!vertexBuffer)
		return 0;

	Vertex* vertex = static_cast< Vertex* >(vertexBuffer->lock());
	T_ASSERT_M (vertex, L"Unable to lock vertex buffer");

	for (uint32_t i = 0; i < m_layers; ++i)
	{
		//float z = i * m_distance;
		for (uint32_t j = 0; j < sizeof_array(c_template); ++j)
		{
			*vertex = c_template[j];
			vertex->layer = float(i);
			//vertex->position[2] += z;
			vertex++;
		}
	}

	vertexBuffer->unlock();

	Ref< render::IndexBuffer > indexBuffer = renderSystem->createIndexBuffer(
		render::ItUInt16,
		m_layers * 6 * sizeof(uint16_t),
		false
	);
	if (!indexBuffer)
		return 0;

	uint16_t* index = static_cast< uint16_t* >(indexBuffer->lock());
	T_ASSERT_M (index, L"Unable to lock index buffer");

	for (uint32_t i = 0; i < m_layers; ++i)
	{
		uint16_t offset = i * sizeof_array(c_template);
		*index++ = 0 + offset;
		*index++ = 1 + offset;
		*index++ = 2 + offset;
		*index++ = 2 + offset;
		*index++ = 3 + offset;
		*index++ = 0 + offset;
	}

	indexBuffer->unlock();

	render::Primitives primitives(
		render::PtTriangles,
		0,
		m_layers * 2,
		0,
		m_layers * sizeof_array(c_template) - 1
	);

	return new PrecipitationComponent(
		vertexBuffer,
		indexBuffer,
		primitives,
		shader
	);
}

void PrecipitationComponentData::serialize(ISerializer& s)
{
	s >> resource::Member< render::Shader >(L"shader", m_shader);
	s >> Member< uint32_t >(L"layers", m_layers, AttributeRange(1));
	s >> Member< float >(L"distance", m_distance, AttributeRange(0.0f));
}

	}
}
