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
#include "Render/Shader/ShaderGraph.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "World/WorldRenderView.h"
#include "World/PostProcess/PostProcess.h"
#include "World/PostProcess/PostProcessStepBokeh.h"

namespace traktor
{
	namespace world
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

const int32_t c_density = 64;

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.PostProcessStepBokeh", 1, PostProcessStepBokeh, PostProcessStep)

PostProcessStepBokeh::PostProcessStepBokeh()
{
}

Ref< PostProcessStepBokeh::Instance > PostProcessStepBokeh::create(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem) const
{
	resource::Proxy< render::Shader > shader = m_shader;
	if (!resourceManager->bind(shader))
		return false;

	std::vector< InstanceBokeh::Source > sources(m_sources.size());
	for (uint32_t i = 0; i < m_sources.size(); ++i)
	{
		sources[i].param = render::getParameterHandle(m_sources[i].param);
		sources[i].paramSize = render::getParameterHandle(m_sources[i].param + L"_Size");
		sources[i].source = render::getParameterHandle(m_sources[i].source);
		sources[i].index = m_sources[i].index;
	}

	// Create dense grid of screen "point quads".
	std::vector< render::VertexElement > vertexElements;
	vertexElements.push_back(render::VertexElement(render::DuPosition, render::DtFloat2, offsetof(Vertex, x)));
	vertexElements.push_back(render::VertexElement(render::DuCustom, render::DtFloat2, offsetof(Vertex, dx)));

	const uint32_t c_quadCount = c_density * c_density;
	const float c_halfDensity = float(c_density) / 2.0f;

	Ref< render::VertexBuffer > vertexBuffer = renderSystem->createVertexBuffer(vertexElements, c_quadCount * 4 * sizeof(Vertex), false);
	
	Random r;

	Vertex* vertex = static_cast< Vertex* >(vertexBuffer->lock());
	for (int32_t y = 0; y < c_density; ++y)
	{
		for (int32_t x = 0; x < c_density; ++x)
		{
			float rx = (r.nextFloat() * 2.0f - 1.0f) / c_density;
			float ry = (r.nextFloat() * 2.0f - 1.0f) / c_density;

			vertex->x = rx + float(x) / c_halfDensity - 1.0f;
			vertex->y = ry + float(y) / c_halfDensity - 1.0f;
			vertex->dx = -1.0f;
			vertex->dy = -1.0f;
			vertex++;

			vertex->x = rx + float(x) / c_halfDensity - 1.0f;
			vertex->y = ry + float(y) / c_halfDensity - 1.0f;
			vertex->dx =  1.0f;
			vertex->dy = -1.0f;
			vertex++;

			vertex->x = rx + float(x) / c_halfDensity - 1.0f;
			vertex->y = ry + float(y) / c_halfDensity - 1.0f;
			vertex->dx =  1.0f;
			vertex->dy =  1.0f;
			vertex++;

			vertex->x = rx + float(x) / c_halfDensity - 1.0f;
			vertex->y = ry + float(y) / c_halfDensity - 1.0f;
			vertex->dx = -1.0f;
			vertex->dy =  1.0f;
			vertex++;
		}
	}
	vertexBuffer->unlock();

	// Create index buffer.
	Ref< render::IndexBuffer > indexBuffer = renderSystem->createIndexBuffer(render::ItUInt32, c_quadCount * 2 * 3 * sizeof(uint32_t), false);

	uint32_t* index = static_cast< uint32_t* >(indexBuffer->lock());
	for (int i = 0; i < c_density; ++i)
	{
		for (int j = 0; j < c_density; ++j)
		{
			uint32_t base = (i * c_density + j) * 4;

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
		sources
	);
}

bool PostProcessStepBokeh::serialize(ISerializer& s)
{
	s >> resource::Member< render::Shader, render::ShaderGraph >(L"shader", m_shader);
	s >> MemberStlVector< Source, MemberComposite< Source > >(L"sources", m_sources);
	return true;
}

PostProcessStepBokeh::Source::Source()
:	index(0)
{
}

bool PostProcessStepBokeh::Source::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"param", param);
	s >> Member< std::wstring >(L"source", source);
	s >> Member< uint32_t >(L"index", index);
	return true;
}

// Instance

PostProcessStepBokeh::InstanceBokeh::InstanceBokeh(
	const resource::Proxy< render::Shader >& shader,
	render::VertexBuffer* vertexBuffer,
	render::IndexBuffer* indexBuffer,
	const std::vector< Source >& sources
)
:	m_shader(shader)
,	m_vertexBuffer(vertexBuffer)
,	m_indexBuffer(indexBuffer)
,	m_sources(sources)
,	m_time(0.0f)
{
	m_handleTime = render::getParameterHandle(L"Time");
	m_handleDeltaTime = render::getParameterHandle(L"DeltaTime");
	m_handleDepthRange = render::getParameterHandle(L"DepthRange");
}

void PostProcessStepBokeh::InstanceBokeh::destroy()
{
}

void PostProcessStepBokeh::InstanceBokeh::render(
	PostProcess* postProcess,
	render::IRenderView* renderView,
	render::ScreenRenderer* screenRenderer,
	const RenderParams& params
)
{
	if (!m_shader.validate())
		return;

	Ref< render::RenderTargetSet > source = postProcess->getTargetRef(render::getParameterHandle(L"InputColor"));
	if (!source)
		return;

	postProcess->prepareShader(m_shader);

	m_shader->setFloatParameter(m_handleTime, m_time);
	m_shader->setFloatParameter(m_handleDeltaTime, params.deltaTime);
	m_shader->setFloatParameter(m_handleDepthRange, params.depthRange);

	for (std::vector< Source >::const_iterator i = m_sources.begin(); i != m_sources.end(); ++i)
	{
		Ref< render::RenderTargetSet > source = postProcess->getTargetRef(i->source);
		if (source)
		{
			m_shader->setTextureParameter(i->param, source->getColorTexture(i->index));
			m_shader->setVectorParameter(i->paramSize, Vector4(
				float(source->getWidth()),
				float(source->getHeight()),
				0.0f,
				0.0f
			));
		}
	}

	renderView->setVertexBuffer(m_vertexBuffer);
	renderView->setIndexBuffer(m_indexBuffer);

	m_shader->draw(renderView, render::Primitives(
		render::PtTriangles,
		0,
		(c_density * c_density) * 2,
		0,
		(c_density * c_density) * 4 - 1
	));

	m_time += params.deltaTime;
}

	}
}
