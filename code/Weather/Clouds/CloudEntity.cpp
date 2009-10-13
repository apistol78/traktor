#include <algorithm>
#include <limits>
#include "Weather/Clouds/CloudEntity.h"
#include "World/WorldRenderer.h"
#include "World/WorldRenderView.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/PrimitiveRenderer.h"
#include "Render/RenderTargetSet.h"
#include "Render/VertexElement.h"
#include "Render/VertexBuffer.h"
#include "Render/IndexBuffer.h"
#include "Render/Context/RenderContext.h"
#include "Core/Math/Vector2.h"
#include "Core/Math/Matrix44.h"

namespace traktor
{
	namespace weather
	{
		namespace
		{

template < typename Comparator >
struct ParticlePredicate
{
	const Vector4& m_direction;

	ParticlePredicate(const Vector4& direction)
	:	m_direction(direction)
	{
	}

	bool operator () (const CloudParticle& p1, const CloudParticle& p2) const
	{
		float d1 = dot3(p1.position, m_direction);
		float d2 = dot3(p2.position, m_direction);
		return Comparator()(d1, d2);
	}
};

struct Vertex
{
	float pos[2];
	float index;
};

const uint32_t c_instanceCount = 16;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.weather.CloudEntity", CloudEntity, world::SpatialEntity)

CloudEntity::CloudEntity()
:	m_handleBillboardView(render::getParameterHandle(L"BillboardView"))
,	m_handleImpostorTarget(render::getParameterHandle(L"ImpostorTarget"))
{
}

bool CloudEntity::create(
	render::IRenderSystem* renderSystem,
	const resource::Proxy< render::Shader >& particleShader,
	const resource::Proxy< render::Shader >& impostorShader,
	uint32_t impostorTargetResolution,
	uint32_t distanceTargetResolution,
	const CloudParticleData& particleData
)
{
	std::vector< render::VertexElement > vertexElements;
	vertexElements.push_back(render::VertexElement(render::DuPosition, render::DtFloat2, offsetof(Vertex, pos)));
	vertexElements.push_back(render::VertexElement(render::DuCustom, render::DtFloat1, offsetof(Vertex, index)));

	m_vertexBuffer = renderSystem->createVertexBuffer(vertexElements, 4 * sizeof(Vertex) * c_instanceCount, false);
	if (!m_vertexBuffer)
		return false;

	Vertex* vertex = static_cast< Vertex* >(m_vertexBuffer->lock());
	T_ASSERT_M (vertex, L"Unable to lock vertex buffer");

	for (uint32_t i = 0; i < c_instanceCount; ++i)
	{
		vertex->pos[0] = -1.0f; vertex->pos[1] =  1.0f; vertex->index = float(i); vertex++;
		vertex->pos[0] = -1.0f; vertex->pos[1] = -1.0f; vertex->index = float(i); vertex++;
		vertex->pos[0] =  1.0f; vertex->pos[1] =  1.0f; vertex->index = float(i); vertex++;
		vertex->pos[0] =  1.0f; vertex->pos[1] = -1.0f; vertex->index = float(i); vertex++;
	}

	m_vertexBuffer->unlock();

	m_indexBuffer = renderSystem->createIndexBuffer(render::ItUInt16, c_instanceCount * 6 * sizeof(uint16_t), false);
	if (!m_indexBuffer)
		return false;

	uint16_t* index = static_cast< uint16_t* >(m_indexBuffer->lock());
	T_ASSERT_M (index, L"Unable to lock index buffer");

	for (uint32_t i = 0; i < c_instanceCount; ++i)
	{
		index[0] = i * 4 + 0;
		index[1] = i * 4 + 2;
		index[2] = i * 4 + 3;
		index[3] = i * 4 + 0;
		index[4] = i * 4 + 3;
		index[5] = i * 4 + 1;
		index += 6;
	}

	m_indexBuffer->unlock();

	render::RenderTargetSetCreateDesc targetDesc;

	targetDesc.count = 1;
	targetDesc.width = impostorTargetResolution;
	targetDesc.height = impostorTargetResolution;
	targetDesc.multiSample = 0;
	targetDesc.depthStencil = false;
	targetDesc.targets[0].format = render::TfR8G8B8A8;

	m_impostorTarget = renderSystem->createRenderTargetSet(targetDesc);
	if (!m_impostorTarget)
		return false;

	m_particleShader = particleShader;
	m_impostorShader = impostorShader;

	if (!m_cluster.create(particleData))
		return false;

	m_particleData = particleData;

	m_transform = Transform::identity();
	m_lastEyePosition.set(0.0f, 0.0f, 0.0f, 0.0f);

	return true;
}

void CloudEntity::render(render::RenderContext* renderContext, const world::WorldRenderView* worldRenderView, render::PrimitiveRenderer* primitiveRenderer)
{
	// Ensure all proxies are validated.
	if (!m_particleShader.validate() || !m_impostorShader.validate())
		return;

	renderCluster(renderContext, worldRenderView, primitiveRenderer, m_cluster);
}

void CloudEntity::update(const world::EntityUpdate* update)
{
}

void CloudEntity::renderCluster(
	render::RenderContext* renderContext,
	const world::WorldRenderView* worldRenderView,
	render::PrimitiveRenderer* primitiveRenderer,
	const CloudParticleCluster& cluster
)
{
	if (worldRenderView->getTechnique() != world::WorldRenderer::getTechniqueDefault())
		return;

	render::IRenderView* renderView = renderContext->getRenderView();
	T_ASSERT (renderView);

	const Vector4& lightDirection = worldRenderView->getLightDirection(0);
	const Aabb& clusterBoundingBox = cluster.getBoundingBox();
	const Matrix44& projection = worldRenderView->getProjection();
	const Matrix44& view = worldRenderView->getView();
	Matrix44 worldView = m_transform.toMatrix44() * view;

	// Calculate billboard view transform.
	Vector4 extents[8];
	clusterBoundingBox.getExtents(extents);

	for (int i = 0; i < 8; ++i)
		extents[i] = worldView * extents[i];

	float minZ = 1e9f;
	float maxZ = -1e9f;
	for (int i = 0; i < 8; ++i)
	{
		minZ = min< float >(minZ, extents[i].z());
		maxZ = max< float >(maxZ, extents[i].z());
	}
	if (maxZ <= 0.0f)
		return;

	const Frustum& cullFrustum = worldRenderView->getCullFrustum();

	const Vector4& c1 = cullFrustum.corners[4];
	float d1 = maxZ / c1.z();
	float clipMinX = c1.x() * d1;
	float clipMaxY = c1.y() * d1;

	const Vector4& c2 = cullFrustum.corners[6];
	float d2 = maxZ / c2.z();
	float clipMaxX = c2.x() * d2;
	float clipMinY = c2.y() * d2;

	float minX = 1e9f, maxX = -1e9f;
	float minY = 1e9f, maxY = -1e9f;

	if (minZ <= 0.0f)
	{
		minX = clipMinX;
		minY = clipMinY;
		maxX = clipMaxX;
		maxY = clipMaxY;
	}
	else
	{
		for (int i = 0; i < 8; ++i)
		{
			float d = maxZ / extents[i].z();
			float x = extents[i].x() * d;
			float y = extents[i].y() * d;
			minX = std::min(minX, x);
			minY = std::min(minY, y);
			maxX = std::max(maxX, x);
			maxY = std::max(maxY, y);
		}

		if (minX < clipMinX)
			minX = clipMinX;
		if (minY < clipMinY)
			minY = clipMinY;
		if (maxX > clipMaxX)
			maxX = clipMaxX;
		if (maxY > clipMaxY)
			maxY = clipMaxY;
	}

	Matrix44 billboardViewTransform =
		translate((maxX + minX) / 2.0f, (maxY + minY) / 2.0f, maxZ) *
		scale((maxX - minX) / 2.0f, (maxY - minY) / 2.0f, 1.0f);

	//// Render billboard debug outline.
	//if (primitiveRenderer)
	//{
	//	primitiveRenderer->pushView(billboardViewTransform);
	//	primitiveRenderer->pushWorld(Matrix44::identity());

	//	for (int i = 0; i < 4; ++i)
	//	{
	//		uint32_t color = 0xff - i * 0x10;
	//		float k = i * 0.05f;

	//		primitiveRenderer->drawWireQuad(
	//			Vector4(-1.0f + k, -1.0f + k, 0.0f, 1.0f),
	//			Vector4( 1.0f - k, -1.0f + k, 0.0f, 1.0f),
	//			Vector4( 1.0f - k,  1.0f - k, 0.0f, 1.0f),
	//			Vector4(-1.0f + k,  1.0f - k, 0.0f, 1.0f),
	//			Color(255, 0, 0, 255)
	//		);
	//	}

	//	primitiveRenderer->popWorld();
	//	primitiveRenderer->popView();
	//}

	Matrix44 clusterProjection = perspectiveLh(PI / 2.0f, 1.0f, 1.0f, 1000.0f);
	Matrix44 clusterView = worldRenderView->getView();
	Matrix44 clusterWorld = m_transform.toMatrix44();

	float scaleX = 2.0f * maxZ / (maxX - minX);
	float scaleY = 2.0f * maxZ / (maxY - minY);

	float offsetX = (maxX + minX) / (2.0f * maxZ);
	float offsetY = (maxY + minY) / (2.0f * maxZ);

	clusterProjection =
		scale(scaleX, scaleY, 1.0f) *
		translate(-offsetX, -offsetY, 0.0f) *
		clusterProjection;

	const Vector4& clusterExtent = clusterBoundingBox.getExtent();
	Scalar clusterMaxExtent = clusterExtent[majorAxis3(clusterExtent)];
	Vector4 clusterCenter = clusterBoundingBox.getCenter();
	Vector4 clusterCameraDirection = clusterWorld.inverse() * clusterView.inverse().axisZ();
	Vector4 eyePosition = view.inverse().translation();
	Vector4 eyeDirection = view.inverse().axisZ();

	// Render particle cloud onto impostor.
	static Vector4 instanceData1[c_instanceCount], instanceData2[c_instanceCount];

	// Render particles into impostor.
	render::ChainRenderBlock* outerRenderBlock = renderContext->alloc< render::ChainRenderBlock >();
	outerRenderBlock->inner = 0;
	outerRenderBlock->next = 0;

	//if ((m_lastEyePosition - eyePosition).length() > 0.1f || abs(dot3(m_lastEyeDirection, eyeDirection)) < 0.9f)
	{
		render::TargetRenderBlock* targetRenderBlock = renderContext->alloc< render::TargetRenderBlock >();
		targetRenderBlock->renderTargetSet = m_impostorTarget;
		targetRenderBlock->renderTargetIndex = 0;
		targetRenderBlock->keepDepthStencil = false;
		targetRenderBlock->clearMask = render::CfColor;
		targetRenderBlock->inner = 0;

		AlignedVector< CloudParticle > particles = cluster.getParticles();
		std::sort(particles.begin(), particles.end(), ParticlePredicate< std::less< float > >(-clusterCameraDirection));

		render::ChainRenderBlock* chainRenderBlockTail = 0;

		uint32_t particleCount = uint32_t(particles.size());
		for (uint32_t i = 0; i < particleCount; )
		{
			uint32_t instanceCount = std::min(particleCount - i, c_instanceCount);

			for (uint32_t j = 0; j < instanceCount; ++j)
			{
				instanceData1[j] = particles[i + j].position;
				instanceData2[j] = Vector4(
					particles[i + j].radius,
					particles[i + j].rotation,
					(particles[i + j].sprite & 3) / 4.0f,
					(particles[i + j].sprite >> 2) / 4.0f
				);
			}

			render::IndexedRenderBlock* particleRenderBlock = renderContext->alloc< render::IndexedRenderBlock >();
			particleRenderBlock->type = render::RbtAlphaBlend;
			particleRenderBlock->distance = 0.0f;
			particleRenderBlock->shader = m_particleShader;
			particleRenderBlock->shaderParams = renderContext->alloc< render::ShaderParameters >();
			particleRenderBlock->indexBuffer = m_indexBuffer;
			particleRenderBlock->vertexBuffer = m_vertexBuffer;
			particleRenderBlock->primitive = render::PtTriangles;
			particleRenderBlock->offset = 0;
			particleRenderBlock->count = instanceCount * 2;
			particleRenderBlock->minIndex = 0;
			particleRenderBlock->maxIndex = c_instanceCount * 4;

			particleRenderBlock->shaderParams->beginParameters(renderContext);
			worldRenderView->setShaderParameters(particleRenderBlock->shaderParams, m_transform.toMatrix44(), Matrix44::identity(), clusterBoundingBox);
			particleRenderBlock->shaderParams->setVectorParameter(L"LightDirection", lightDirection);
			particleRenderBlock->shaderParams->setVectorParameter(L"Eye", eyePosition);
			particleRenderBlock->shaderParams->setFloatParameter(L"ParticleDensity", m_particleData.getDensity());
			particleRenderBlock->shaderParams->setMatrixParameter(L"ClusterProjection", clusterProjection);
			particleRenderBlock->shaderParams->setMatrixParameter(L"ClusterView", clusterView);
			particleRenderBlock->shaderParams->setMatrixParameter(L"ClusterWorld", clusterWorld);
			particleRenderBlock->shaderParams->setVectorArrayParameter(L"InstanceData1", instanceData1, instanceCount);
			particleRenderBlock->shaderParams->setVectorArrayParameter(L"InstanceData2", instanceData2, instanceCount);
			particleRenderBlock->shaderParams->endParameters(renderContext);

			render::ChainRenderBlock* chainRenderBlock = renderContext->alloc< render::ChainRenderBlock >();
			chainRenderBlock->inner = particleRenderBlock;
			chainRenderBlock->next = 0;

			if (chainRenderBlockTail)
			{
				chainRenderBlockTail->next = chainRenderBlock;
				chainRenderBlockTail = chainRenderBlock;
			}
			else
			{
				targetRenderBlock->inner = chainRenderBlock;
				chainRenderBlockTail = chainRenderBlock;
			}

			i += instanceCount;
		}

		outerRenderBlock->inner = targetRenderBlock;

		m_lastEyePosition = eyePosition;
		m_lastEyeDirection = eyeDirection;
	}

	// Render impostor.
	render::NonIndexedRenderBlock* renderBlock = renderContext->alloc< render::NonIndexedRenderBlock >();

	renderBlock->type = render::RbtAlphaBlend;
	renderBlock->distance = 0.0f;
	renderBlock->shader = m_impostorShader;
	renderBlock->shaderParams = renderContext->alloc< render::ShaderParameters >();
	renderBlock->vertexBuffer = m_vertexBuffer;
	renderBlock->primitive = render::PtTriangleStrip;
	renderBlock->offset = 0;
	renderBlock->count = 2;

	renderBlock->shaderParams->beginParameters(renderContext);
	renderBlock->shaderParams->setMatrixParameter(m_handleBillboardView, billboardViewTransform);
	renderBlock->shaderParams->setSamplerTexture(m_handleImpostorTarget, m_impostorTarget->getColorTexture(0));
	worldRenderView->setShaderParameters(renderBlock->shaderParams);
	renderBlock->shaderParams->endParameters(renderContext);

	// Enqueue blocks.
	outerRenderBlock->type = render::RbtAlphaBlend;
	outerRenderBlock->distance = -std::numeric_limits< float >::max();	// Ensure block is rendered last of everything else.
	outerRenderBlock->next = renderBlock;
	renderContext->draw(outerRenderBlock);
}

void CloudEntity::setTransform(const Transform& transform)
{
	m_transform = transform;
}

bool CloudEntity::getTransform(Transform& outTransform) const
{
	outTransform = m_transform;
	return true;
}

Aabb CloudEntity::getBoundingBox() const
{
	return m_cluster.getBoundingBox();
}

	}
}
