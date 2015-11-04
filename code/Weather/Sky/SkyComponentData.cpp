#include <cmath>
#include "Core/Math/Const.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Resource/IResourceManager.h"
#include "Render/IRenderSystem.h"
#include "Render/VertexElement.h"
#include "Render/VertexBuffer.h"
#include "Render/IndexBuffer.h"
#include "Render/Shader.h"
#include "Resource/Member.h"
#include "Weather/Sky/SkyComponent.h"
#include "Weather/Sky/SkyComponentData.h"

namespace traktor
{
	namespace weather
	{
		namespace
		{

const int c_longitudes = 16;
const int c_latitudes = 24;
const int c_vertexCount = (c_longitudes + 1) * c_latitudes;
const int c_triangleCount = ((c_latitudes - 1) * ((c_longitudes + 1) * 2));
const int c_indexCount = c_triangleCount * 3;

		}

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.weather.SkyComponentData", 0, SkyComponentData, world::IEntityComponentData)

SkyComponentData::SkyComponentData()
:	m_sunDirection(0.0f, 1.0f, 0.0f, 0.0f)
,	m_offset(0.0f)
{
}

Ref< SkyComponent > SkyComponentData::createComponent(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem) const
{
	resource::Proxy< render::Shader > shader;
	if (!resourceManager->bind(m_shader, shader))
		return 0;

	std::vector< render::VertexElement > vertexElements;
	vertexElements.push_back(render::VertexElement(render::DuPosition, render::DtFloat2, 0));

	Ref< render::VertexBuffer > vertexBuffer = renderSystem->createVertexBuffer(
		vertexElements,
		c_vertexCount * sizeof(float) * 2,
		false
	);
	if (!vertexBuffer)
		return 0;

	float* vertex = static_cast< float* >(vertexBuffer->lock());
	T_ASSERT_M (vertex, L"Unable to lock vertex buffer");
	
	for (int i = 0; i < c_latitudes; ++i)
	{
		float phi = float(i) / (c_latitudes - 1);
		for (int j = 0; j <= c_longitudes; ++j)
		{
			float theta = float(j) / c_longitudes;
			*vertex++ = phi;
			*vertex++ = theta;
		}
	}

	vertexBuffer->unlock();

	Ref< render::IndexBuffer > indexBuffer = renderSystem->createIndexBuffer(
		render::ItUInt16,
		c_indexCount * sizeof(uint16_t),
		false
	);
	if (!indexBuffer)
		return 0;

	uint16_t* index = static_cast< uint16_t* >(indexBuffer->lock());
	T_ASSERT_M (index, L"Unable to lock index buffer");

	for (int k = 0; k < c_latitudes - 1; ++k)
	{
		int o = k * (c_longitudes + 1);
		for (int i = 0, j = c_longitudes; i <= c_longitudes; j = i, ++i)
		{
			*index++ = o + i;
			*index++ = o + i + c_longitudes + 1;
			*index++ = o + j;
			*index++ = o + j;
			*index++ = o + i + c_longitudes + 1;
			*index++ = o + j + c_longitudes + 1;
		}
	}

	indexBuffer->unlock();

	render::Primitives primitives(
		render::PtTriangles,
		0,
		c_triangleCount,
		0,
		c_vertexCount - 1
	);

	return new SkyComponent(
		vertexBuffer,
		indexBuffer,
		primitives,
		shader,
		m_sunDirection.xyz0().normalized(),
		m_offset
	);
}

void SkyComponentData::serialize(ISerializer& s)
{
	s >> resource::Member< render::Shader >(L"shader", m_shader);
	s >> Member< Vector4 >(L"sunDirection", m_sunDirection);
	s >> Member< float >(L"offset", m_offset);
}

	}
}
