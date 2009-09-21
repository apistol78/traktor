#include "World/PostProcess/PostProcessStepSsao.h"
#include "World/PostProcess/PostProcess.h"
#include "World/WorldRenderView.h"
#include "Render/IRenderSystem.h"
#include "Render/ScreenRenderer.h"
#include "Render/Shader.h"
#include "Render/ShaderGraph.h"
#include "Render/RenderTargetSet.h"
#include "Render/ISimpleTexture.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Math/Const.h"
#include "Core/Math/RandomGeometry.h"
#include "Core/Misc/AutoPtr.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.world.PostProcessStepSsao", PostProcessStepSsao, PostProcessStep)

bool PostProcessStepSsao::create(PostProcess* postProcess, resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem)
{
	if (!resourceManager->bind(m_shader))
		return false;

	RandomGeometry random;
	for (int i = 0; i < sizeof_array(m_offsets); ++i)
	{
		float r = random.nextFloat() * (1.0f - 0.1f) + 0.1f;
		m_offsets[i] = random.nextUnit() * Scalar(r);
	}

	AutoArrayPtr< uint8_t > data(new uint8_t [128 * 128 * 4]);
	for (uint32_t y = 0; y < 128; ++y)
	{
		for (uint32_t x = 0; x < 128; ++x)
		{
			Vector4 normal = random.nextUnit() * Scalar(0.5f) + Scalar(0.5f);
			data[(x + y * 128) * 4 + 0] = uint8_t(normal.x() * 255);
			data[(x + y * 128) * 4 + 1] = uint8_t(normal.y() * 255);
			data[(x + y * 128) * 4 + 2] = uint8_t(normal.z() * 255);
			data[(x + y * 128) * 4 + 3] = 0;
		}
	}

	render::SimpleTextureCreateDesc desc;
	desc.width = 128;
	desc.height = 128;
	desc.mipCount = 1;
	desc.format = render::TfR8G8B8A8;
	desc.immutable = true;
	desc.initialData[0].data = data.ptr();
	desc.initialData[0].pitch = 128 * 4;
	desc.initialData[0].slicePitch = 0;

	m_randomNormals = renderSystem->createSimpleTexture(desc);
	if (!m_randomNormals)
		return false;

	return true;
}

void PostProcessStepSsao::destroy(PostProcess* postProcess)
{
	if (m_randomNormals)
	{
		m_randomNormals->destroy();
		m_randomNormals = 0;
	}
}

void PostProcessStepSsao::render(
	PostProcess* postProcess,
	const WorldRenderView& worldRenderView,
	render::IRenderView* renderView,
	render::ScreenRenderer* screenRenderer,
	float deltaTime
)
{
	if (!m_shader.validate())
		return;

	Ref< render::RenderTargetSet > sourceColor = postProcess->getTargetRef(1);
	Ref< render::RenderTargetSet > sourceDepth = postProcess->getTargetRef(2);
	if (!sourceColor || !sourceDepth)
		return;

	postProcess->prepareShader(m_shader);

	Vector4 sourceColorSize(
		float(sourceColor->getWidth()),
		float(sourceColor->getHeight()),
		0.0f,
		0.0f
	);
	Vector4 sourceDepthSize(
		float(sourceDepth->getWidth()),
		float(sourceDepth->getHeight()),
		0.0f,
		0.0f
	);

	const Frustum& viewFrustum = worldRenderView.getViewFrustum();
	Vector4 viewEdgeTopLeft = viewFrustum.corners[4];
	Vector4 viewEdgeTopRight = viewFrustum.corners[5];
	Vector4 viewEdgeBottomLeft = viewFrustum.corners[7];
	Vector4 viewEdgeBottomRight = viewFrustum.corners[6];

	const Matrix44& projection = worldRenderView.getProjection();

	m_shader->setSamplerTexture(L"Frame", sourceColor->getColorTexture(0));
	m_shader->setSamplerTexture(L"Depth", sourceDepth->getColorTexture(0));
	m_shader->setVectorParameter(L"Frame_Size", sourceColorSize);
	m_shader->setVectorParameter(L"Depth_Size", sourceDepthSize);
	m_shader->setVectorParameter(L"ViewEdgeTopLeft", viewEdgeTopLeft);
	m_shader->setVectorParameter(L"ViewEdgeTopRight", viewEdgeTopRight);
	m_shader->setVectorParameter(L"ViewEdgeBottomLeft", viewEdgeBottomLeft);
	m_shader->setVectorParameter(L"ViewEdgeBottomRight", viewEdgeBottomRight);
	m_shader->setMatrixParameter(L"Projection", projection);
	m_shader->setVectorArrayParameter(L"Offsets", m_offsets, sizeof_array(m_offsets));
	m_shader->setSamplerTexture(L"RandomNormals", m_randomNormals);

	screenRenderer->draw(renderView, m_shader);
}

bool PostProcessStepSsao::serialize(Serializer& s)
{
	s >> resource::Member< render::Shader, render::ShaderGraph >(L"shader", m_shader);
	return true;
}

	}
}
