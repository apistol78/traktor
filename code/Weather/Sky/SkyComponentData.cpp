/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cmath>
#include "Core/Math/Const.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Resource/IResourceManager.h"
#include "Render/IRenderSystem.h"
#include "Render/ITexture.h"
#include "Render/Buffer.h"
#include "Render/Shader.h"
#include "Render/VertexElement.h"
#include "Resource/Member.h"
#include "Weather/Sky/SkyComponent.h"
#include "Weather/Sky/SkyComponentData.h"

namespace traktor::weather
{
	namespace
	{

const resource::Id< render::Shader > c_defaultShader(Guid(L"{4CF929EB-3A8B-C340-AA0A-0C5C80625BF1}"));
const resource::Id< render::ITexture > c_defaultTexture(Guid(L"{93E6996B-8903-4AD0-811A-C8C03C8E38C6}"));
const int32_t c_longitudes = 16;
const int32_t c_latitudes = 24;
const int32_t c_vertexCount = (c_longitudes + 1) * c_latitudes;
const int32_t c_triangleCount = ((c_latitudes - 1) * ((c_longitudes + 1) * 2));
const int32_t c_indexCount = c_triangleCount * 3;

	}

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.weather.SkyComponentData", 3, SkyComponentData, world::IEntityComponentData)

SkyComponentData::SkyComponentData()
:	m_shader(c_defaultShader)
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
	vertexElements.push_back(render::VertexElement(render::DataUsage::Position, render::DtFloat2, 0));
	Ref< const render::IVertexLayout > vertexLayout = renderSystem->createVertexLayout(vertexElements);

	Ref< render::Buffer > vertexBuffer = renderSystem->createBuffer(
		render::BuVertex,
		c_vertexCount,
		sizeof(float) * 2,
		false
	);
	if (!vertexBuffer)
		return nullptr;

	float* vertex = static_cast< float* >(vertexBuffer->lock());
	if (!vertex)
		return nullptr;

	for (int32_t i = 0; i < c_latitudes; ++i)
	{
		const float phi = float(i) / (c_latitudes - 1);
		for (int32_t j = 0; j <= c_longitudes; ++j)
		{
			const float theta = float(j) / c_longitudes;
			*vertex++ = phi;
			*vertex++ = theta;
		}
	}

	vertexBuffer->unlock();

	Ref< render::Buffer > indexBuffer = renderSystem->createBuffer(
		render::BuIndex,
		c_indexCount,
		sizeof(uint16_t),
		false
	);
	if (!indexBuffer)
		return nullptr;

	uint16_t* index = static_cast< uint16_t* >(indexBuffer->lock());
	if (!index)
		return nullptr;

	for (int32_t k = 0; k < c_latitudes - 1; ++k)
	{
		const int32_t o = k * (c_longitudes + 1);
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
		render::PrimitiveType::Triangles,
		0,
		c_triangleCount,
		0,
		c_vertexCount - 1
	);

	return new SkyComponent(
		vertexLayout,
		vertexBuffer,
		indexBuffer,
		primitives,
		shader,
		texture
	);
}

void SkyComponentData::setTransform(const world::EntityData* owner, const Transform& transform)
{
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

	if (s.getVersion< SkyComponentData >() < 3)
	{
		float dummy;
		s >> Member< float >(L"offset", dummy);
	}
}

}
