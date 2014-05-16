#include "Core/Math/Random.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Core/Serialization/MemberComposite.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/RenderTargetSet.h"
#include "Render/Shader.h"
#include "Render/VertexBuffer.h"
#include "Render/VertexElement.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "World/PostProcess/PostProcess.h"
#include "World/PostProcess/PostProcessStepLensDirt.h"

namespace traktor
{
	namespace world
	{
		namespace
		{

struct Vertex
{
	float position[2];
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.PostProcessStepLensDirt", 0, PostProcessStepLensDirt, PostProcessStep)

PostProcessStepLensDirt::PostProcessStepLensDirt()
{
}

Ref< PostProcessStep::Instance > PostProcessStepLensDirt::create(
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	uint32_t width,
	uint32_t height
) const
{
	Ref< InstanceLensDirt > instance = new InstanceLensDirt(this);

	if (!resourceManager->bind(m_shader, instance->m_shader))
		return 0;

	instance->m_sources.resize(m_sources.size());
	for (uint32_t i = 0; i < m_sources.size(); ++i)
	{
		instance->m_sources[i].param = render::getParameterHandle(m_sources[i].param);
		instance->m_sources[i].source = render::getParameterHandle(m_sources[i].source);
		instance->m_sources[i].index = m_sources[i].index;
	}

	std::vector< render::VertexElement > vertexElements;
	vertexElements.push_back(render::VertexElement(render::DuPosition, render::DtFloat2, offsetof(Vertex, position), 0));
	T_ASSERT_M (render::getVertexSize(vertexElements) == sizeof(Vertex), L"Incorrect size of vertex");

	instance->m_vertexBuffer = renderSystem->createVertexBuffer(vertexElements, 4 * sizeof(Vertex), false);

	Vertex* vertex = static_cast< Vertex* >(instance->m_vertexBuffer->lock());
	T_ASSERT (vertex);

	vertex->position[0] = -1.0f; vertex->position[1] = -1.0f; ++vertex;
	vertex->position[0] = -1.0f; vertex->position[1] =  1.0f; ++vertex;
	vertex->position[0] =  1.0f; vertex->position[1] = -1.0f; ++vertex;
	vertex->position[0] =  1.0f; vertex->position[1] =  1.0f; ++vertex;

	instance->m_vertexBuffer->unlock();

	instance->m_primitives.setNonIndexed(render::PtTriangleStrip, 0, 2);

	Random random;
	for (uint32_t i = 0; i < InstanceLensDirt::InstanceCount; ++i)
	{
		float x = random.nextFloat() * 2.0f - 1.0f;
		float y = random.nextFloat() * 2.0f - 1.0f;
		float s = traktor::sqrtf(x * x + y * y) / 1.414214f;
		float w = random.nextFloat() * (s * 0.8f) + 0.2f;
		instance->m_instances[i] = Vector4(
			x,
			y,
			random.nextFloat() * TWO_PI,
			w * w * 1.3f
		);
	}

	return instance;
}

void PostProcessStepLensDirt::serialize(ISerializer& s)
{
	s >> resource::Member< render::Shader >(L"shader", m_shader);
	s >> MemberStlVector< Source, MemberComposite< Source > >(L"sources", m_sources);
}

PostProcessStepLensDirt::Source::Source()
:	index(0)
{
}

void PostProcessStepLensDirt::Source::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"param", param);
	s >> Member< std::wstring >(L"source", source);
	s >> Member< uint32_t >(L"index", index);
}

// Instance

PostProcessStepLensDirt::InstanceLensDirt::InstanceLensDirt(const PostProcessStepLensDirt* step)
:	m_step(step)
{
	m_handleInstances = render::getParameterHandle(L"Instances");
}

void PostProcessStepLensDirt::InstanceLensDirt::destroy()
{
	safeDestroy(m_vertexBuffer);
}

void PostProcessStepLensDirt::InstanceLensDirt::render(
	PostProcess* postProcess,
	render::IRenderView* renderView,
	render::ScreenRenderer* screenRenderer,
	const RenderParams& params
)
{
	postProcess->prepareShader(m_shader);

	m_shader->setVectorArrayParameter(m_handleInstances, m_instances, InstanceCount);

	for (std::vector< Source >::const_iterator i = m_sources.begin(); i != m_sources.end(); ++i)
	{
		render::RenderTargetSet* source = postProcess->getTarget(i->source);
		if (source)
			m_shader->setTextureParameter(i->param, source->getColorTexture(i->index));
	}

	m_shader->draw(
		renderView,
		m_vertexBuffer,
		0,
		m_primitives,
		InstanceCount
	);
}

	}
}
