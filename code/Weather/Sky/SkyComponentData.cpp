#include <cmath>
#include "Core/Math/Const.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Resource/IResourceManager.h"
#include "Render/IRenderSystem.h"
#include "Render/ITexture.h"
#include "Render/IndexBuffer.h"
#include "Render/Shader.h"
#include "Render/VertexBuffer.h"
#include "Render/VertexElement.h"
#include "Resource/Member.h"
#include "Weather/Sky/SkyComponent.h"
#include "Weather/Sky/SkyComponentData.h"

namespace traktor
{
	namespace weather
	{
		namespace
		{

const Guid c_defaultShader(L"{4CF929EB-3A8B-C340-AA0A-0C5C80625BF1}");
const Guid c_defaultTexture(L"{93E6996B-8903-4AD0-811A-C8C03C8E38C6}");
const int32_t c_longitudes = 16;
const int32_t c_latitudes = 24;
const int32_t c_vertexCount = (c_longitudes + 1) * c_latitudes;
const int32_t c_triangleCount = ((c_latitudes - 1) * ((c_longitudes + 1) * 2));
const int32_t c_indexCount = c_triangleCount * 3;

		}

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.weather.SkyComponentData", 2, SkyComponentData, world::IEntityComponentData)

SkyComponentData::SkyComponentData()
:	m_offset(0.0f)
,	m_shader(c_defaultShader)
,	m_texture(c_defaultTexture)
{
}

Ref< SkyComponent > SkyComponentData::createComponent(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem) const
{
	resource::Proxy< render::Shader > shader;
	if (!resourceManager->bind(m_shader, shader))
		return nullptr;
		
	resource::Proxy< render::ITexture > texture;
	if (m_texture.isValid())
	{
		if (!resourceManager->bind(m_texture, texture))
			return nullptr;
	}

	AlignedVector< render::VertexElement > vertexElements;
	vertexElements.push_back(render::VertexElement(render::DuPosition, render::DtFloat2, 0));

	Ref< render::VertexBuffer > vertexBuffer = renderSystem->createVertexBuffer(
		vertexElements,
		c_vertexCount * sizeof(float) * 2,
		false
	);
	if (!vertexBuffer)
		return nullptr;

	float* vertex = static_cast< float* >(vertexBuffer->lock());
	if (!vertex)
		return nullptr;

	for (int32_t i = 0; i < c_latitudes; ++i)
	{
		float phi = float(i) / (c_latitudes - 1);
		for (int32_t j = 0; j <= c_longitudes; ++j)
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
		return nullptr;

	uint16_t* index = static_cast< uint16_t* >(indexBuffer->lock());
	if (!index)
		return nullptr;

	for (int32_t k = 0; k < c_latitudes - 1; ++k)
	{
		int32_t o = k * (c_longitudes + 1);
		for (int32_t i = 0, j = c_longitudes; i <= c_longitudes; j = i, ++i)
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
		texture,
		m_offset
	);
}

void SkyComponentData::serialize(ISerializer& s)
{
	s >> resource::Member< render::Shader >(L"shader", m_shader);

	if (s.getVersion< SkyComponentData >() >= 2)
		s >> resource::Member< render::ITexture >(L"texture", m_texture);

	if (s.getVersion< SkyComponentData >() < 1)
	{
		Vector4 dummy;
		s >> Member< Vector4 >(L"sunDirection", dummy);
	}

	s >> Member< float >(L"offset", m_offset);
}

	}
}
