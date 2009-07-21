#include <cmath>
#include "Weather/Sky/SkyEntityData.h"
#include "Weather/Sky/SkyEntity.h"
#include "Resource/IResourceManager.h"
#include "Render/IRenderSystem.h"
#include "Render/VertexElement.h"
#include "Render/VertexBuffer.h"
#include "Render/IndexBuffer.h"
#include "Render/Shader.h"
#include "Render/ShaderGraph.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Math/Const.h"
#include "Resource/Member.h"

namespace traktor
{
	namespace weather
	{
		namespace
		{

const int c_longitudes = 16;
const int c_latitudes = 8;
const int c_vertexCount = (c_longitudes * (c_latitudes - 1) + 2);
const int c_triangleCount = (c_latitudes - 2) * (c_longitudes * 2) + c_longitudes * 2;
const int c_indexCount = c_triangleCount * 3;

		}

T_IMPLEMENT_RTTI_EDITABLE_CLASS(L"traktor.weather.SkyEntityData", SkyEntityData, world::EntityData)

SkyEntityData::SkyEntityData()
{
}

SkyEntity* SkyEntityData::createEntity(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem) const
{
	if (!resourceManager->bind(m_shader))
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
	
	for (int i = 1; i < c_latitudes; ++i)
	{
		float phi = float(i) / (c_latitudes - 1);
		for (int j = 0; j < c_longitudes; ++j)
		{
			float theta = float(j) / c_longitudes;
			*vertex++ = phi;
			*vertex++ = theta;
		}
	}

	*vertex++ = 0.0f;	// Top
	*vertex++ = 0.0f;

	*vertex++ = 2.0f;	// Bottom
	*vertex++ = 0.0f;

	vertexBuffer->unlock();

	Ref< render::IndexBuffer > indexBuffer = renderSystem->createIndexBuffer(
		render::ItUInt16,
		c_indexCount * sizeof(unsigned short),
		false
	);
	if (!indexBuffer)
		return 0;

	unsigned short* index = static_cast< unsigned short* >(indexBuffer->lock());
	T_ASSERT_M (index, L"Unable to lock index buffer");

	for (int k = 0; k < c_latitudes - 2; ++k)
	{
		int o = k * c_longitudes;
		for (int i = 0, j = c_longitudes - 1; i < c_longitudes; j = i, ++i)
		{
			*index++ = o + i;
			*index++ = o + i + c_longitudes;
			*index++ = o + j;
			*index++ = o + j;
			*index++ = o + i + c_longitudes;
			*index++ = o + j + c_longitudes;
		}
	}

	for (int i = 0, j = c_longitudes - 1; i < c_longitudes; j = i, ++i)
	{
		*index++ = j;
		*index++ = c_vertexCount - 2;
		*index++ = i;
	}

	for (int i = 0, j = c_longitudes - 1; i < c_longitudes; j = i, ++i)
	{
		*index++ = c_longitudes * (c_latitudes - 2) + i;
		*index++ = c_vertexCount - 1;
		*index++ = c_longitudes * (c_latitudes - 2) + j;
	}

	indexBuffer->unlock();

	render::Primitives primitives(
		render::PtTriangles,
		0,
		c_triangleCount,
		0,
		c_vertexCount - 1
	);

	return gc_new< SkyEntity >(
		vertexBuffer,
		indexBuffer,
		primitives,
		m_shader
	);
}

bool SkyEntityData::serialize(Serializer& s)
{
	return s >> resource::Member< render::Shader, render::ShaderGraph >(L"shader", m_shader);
}

	}
}
