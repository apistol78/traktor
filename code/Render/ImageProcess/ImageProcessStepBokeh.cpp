/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Math/Const.h"
#include "Core/Math/Random.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Core/Serialization/MemberComposite.h"
#include "Render/IndexBuffer.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/RenderTargetSet.h"
#include "Render/Shader.h"
#include "Render/VertexBuffer.h"
#include "Render/VertexElement.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "World/WorldRenderView.h"
#include "Render/ImageProcess/ImageProcess.h"
#include "Render/ImageProcess/ImageProcessStepBokeh.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

#pragma pack(1)
struct Vertex
{
	float x;
	float y;
	float dx;
	float dy;
};
#pragma pack()

const int32_t c_density = 5;

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ImageProcessStepBokeh", 1, ImageProcessStepBokeh, ImageProcessStep)

ImageProcessStepBokeh::ImageProcessStepBokeh()
{
}

Ref< ImageProcessStepBokeh::Instance > ImageProcessStepBokeh::create(
	resource::IResourceManager* resourceManager,
	IRenderSystem* renderSystem,
	uint32_t width,
	uint32_t height
) const
{
	resource::Proxy< Shader > shader;
	if (!resourceManager->bind(m_shader, shader))
		return 0;

	std::vector< InstanceBokeh::Source > sources(m_sources.size());
	for (uint32_t i = 0; i < m_sources.size(); ++i)
	{
		sources[i].param = getParameterHandle(m_sources[i].param);
		sources[i].source = getParameterHandle(m_sources[i].source);
		sources[i].index = m_sources[i].index;
	}

	// Create dense grid of screen "point quads".
	std::vector< VertexElement > vertexElements;
	vertexElements.push_back(VertexElement(DuPosition, DtFloat2, offsetof(Vertex, x)));
	vertexElements.push_back(VertexElement(DuCustom, DtFloat2, offsetof(Vertex, dx)));

	uint32_t quadWidth = width / c_density;
	uint32_t quadHeight = height / c_density;
	uint32_t quadCount = quadWidth * quadHeight;

	Ref< VertexBuffer > vertexBuffer = renderSystem->createVertexBuffer(vertexElements, quadCount * 4 * sizeof(Vertex), false);
	
	float ox = float(c_density) / width;
	float oy = float(c_density) / height;
	Random r;

	Vertex* vertex = static_cast< Vertex* >(vertexBuffer->lock());
	for (uint32_t y = 0; y < quadHeight; ++y)
	{
		for (uint32_t x = 0; x < quadWidth; ++x)
		{
			vertex->x = ox + float(2.0f * x) / quadWidth - 1.0f;
			vertex->y = oy + float(2.0f * y) / quadHeight - 1.0f;
			vertex->dx = -1.0f;
			vertex->dy = -1.0f;
			vertex++;

			vertex->x = ox + float(2.0f * x) / quadWidth - 1.0f;
			vertex->y = oy + float(2.0f * y) / quadHeight - 1.0f;
			vertex->dx =  1.0f;
			vertex->dy = -1.0f;
			vertex++;

			vertex->x = ox + float(2.0f * x) / quadWidth - 1.0f;
			vertex->y = oy + float(2.0f * y) / quadHeight - 1.0f;
			vertex->dx =  1.0f;
			vertex->dy =  1.0f;
			vertex++;

			vertex->x = ox + float(2.0f * x) / quadWidth - 1.0f;
			vertex->y = oy + float(2.0f * y) / quadHeight - 1.0f;
			vertex->dx = -1.0f;
			vertex->dy =  1.0f;
			vertex++;
		}
	}
	vertexBuffer->unlock();

	// Create index buffer.
	Ref< IndexBuffer > indexBuffer = renderSystem->createIndexBuffer(ItUInt32, quadCount * 2 * 3 * sizeof(uint32_t), false);

	uint32_t* index = static_cast< uint32_t* >(indexBuffer->lock());
	for (uint32_t i = 0; i < quadHeight; ++i)
	{
		for (uint32_t j = 0; j < quadWidth; ++j)
		{
			uint32_t base = (i * quadWidth + j) * 4;

			*index++ = base + 0;
			*index++ = base + 1;
			*index++ = base + 3;

			*index++ = base + 1;
			*index++ = base + 2;
			*index++ = base + 3;
		}
	}
	indexBuffer->unlock();

	return new InstanceBokeh(
		shader,
		vertexBuffer,
		indexBuffer,
		sources,
		quadCount
	);
}

void ImageProcessStepBokeh::serialize(ISerializer& s)
{
	s >> resource::Member< Shader >(L"shader", m_shader);
	s >> MemberStlVector< Source, MemberComposite< Source > >(L"sources", m_sources);
}

ImageProcessStepBokeh::Source::Source()
:	index(0)
{
}

void ImageProcessStepBokeh::Source::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"param", param);
	s >> Member< std::wstring >(L"source", source);
	s >> Member< uint32_t >(L"index", index);
}

// Instance

ImageProcessStepBokeh::InstanceBokeh::InstanceBokeh(
	const resource::Proxy< Shader >& shader,
	VertexBuffer* vertexBuffer,
	IndexBuffer* indexBuffer,
	const std::vector< Source >& sources,
	uint32_t quadCount
)
:	m_shader(shader)
,	m_vertexBuffer(vertexBuffer)
,	m_indexBuffer(indexBuffer)
,	m_sources(sources)
,	m_quadCount(quadCount)
,	m_time(0.0f)
{
	m_handleTime = getParameterHandle(L"Time");
	m_handleDeltaTime = getParameterHandle(L"DeltaTime");
	m_handleRatio = getParameterHandle(L"Ratio");
}

void ImageProcessStepBokeh::InstanceBokeh::destroy()
{
}

void ImageProcessStepBokeh::InstanceBokeh::render(
	ImageProcess* imageProcess,
	IRenderView* renderView,
	ScreenRenderer* screenRenderer,
	const RenderParams& params
)
{
	imageProcess->prepareShader(m_shader);

	m_shader->setFloatParameter(m_handleTime, m_time);
	m_shader->setFloatParameter(m_handleDeltaTime, params.deltaTime);
	m_shader->setFloatParameter(m_handleRatio, float(renderView->getWidth()) / renderView->getHeight());

	for (std::vector< Source >::const_iterator i = m_sources.begin(); i != m_sources.end(); ++i)
	{
		RenderTargetSet* source = imageProcess->getTarget(i->source);
		if (source)
			m_shader->setTextureParameter(i->param, source->getColorTexture(i->index));
	}

	m_shader->draw(
		renderView,
		m_vertexBuffer,
		m_indexBuffer,
		Primitives(
			PtTriangles,
			0,
			m_quadCount * 2,
			0,
			m_quadCount * 4 - 1
		)
	);

	m_time += params.deltaTime;
}

	}
}
