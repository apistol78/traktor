#include "Heightfield/Heightfield.h"
#include "Render/IndexBuffer.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/ISimpleTexture.h"
#include "Render/ScreenRenderer.h"
#include "Render/Shader.h"
#include "Render/VertexBuffer.h"
#include "Render/VertexElement.h"
#include "Render/Context/RenderContext.h"
#include "Resource/IResourceManager.h"
#include "Terrain/OceanEntity.h"
#include "Terrain/OceanEntityData.h"
#include "World/IWorldRenderPass.h"
#include "World/WorldRenderView.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.OceanEntity", OceanEntity, world::Entity)

OceanEntity::OceanEntity()
:	m_altitude(0.0f)
{
}

bool OceanEntity::create(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem, const OceanEntityData& data)
{
	const uint32_t gridSize = 100;
	const uint32_t vertexCount = gridSize * gridSize;
	const uint32_t triangleCount = (gridSize - 1) * (gridSize - 1) * 2;
	const uint32_t indexCount = triangleCount * 3;

	std::vector< render::VertexElement > vertexElements;
	vertexElements.push_back(render::VertexElement(render::DuPosition, render::DtFloat2, 0));

	m_vertexBuffer = renderSystem->createVertexBuffer(
		vertexElements,
		vertexCount * sizeof(float) * 2,
		false
	);
	if (!m_vertexBuffer)
		return 0;

	float* vertex = static_cast< float* >(m_vertexBuffer->lock());
	T_ASSERT_M (vertex, L"Unable to lock vertex buffer");

	for (int y = 0; y < gridSize; ++y)
	{
		float fy = 2.0f * float(y) / (gridSize - 1) - 1.0f;
		for (int x = 0; x < gridSize; ++x)
		{
			float fx = 2.0f * float(x) / (gridSize - 1) - 1.0f;
			*vertex++ = fx;
			*vertex++ = fy;
		}
	}

	m_vertexBuffer->unlock();

	m_indexBuffer = renderSystem->createIndexBuffer(
		render::ItUInt32,
		indexCount * sizeof(uint32_t),
		false
	);
	if (!m_indexBuffer)
		return 0;

	uint32_t* index = static_cast< uint32_t* >(m_indexBuffer->lock());
	T_ASSERT_M (index, L"Unable to lock index buffer");

	for (int i = 0; i < gridSize - 1; ++i)
	{
		for (int j = 0; j < gridSize - 1; ++j)
		{
			#define IDX(ii, jj) uint32_t((ii) + (jj) * gridSize)

			*index++ = IDX(i + 1, j + 1);
			*index++ = IDX(i    , j + 1);
			*index++ = IDX(i    , j    );
			*index++ = IDX(i + 1, j    );
			*index++ = IDX(i + 1, j + 1);
			*index++ = IDX(i    , j    );
		}
	}

	m_indexBuffer->unlock();

	m_primitives.setIndexed(
		render::PtTriangles,
		0,
		triangleCount,
		0,
		indexCount - 1
	);

	m_screenRenderer = new render::ScreenRenderer();
	if (!m_screenRenderer->create(renderSystem))
		return 0;

	m_heightfield = data.m_heightfield;
	m_shader = data.m_shader;
	m_altitude = data.m_altitude;

	if (!resourceManager->bind(m_heightfield))
		return false;
	if (!resourceManager->bind(m_shader))
		return false;

	for (int i = 0; i < MaxWaves; ++i)
		m_waveData[i] = Vector4(data.m_waves[i].direction.x, data.m_waves[i].direction.y, data.m_waves[i].amplitude, data.m_waves[i].phase);

	return true;
}

void OceanEntity::render(
	render::RenderContext* renderContext,
	world::WorldRenderView& worldRenderView,
	world::IWorldRenderPass& worldRenderPass
)
{
	if (!m_shader.validate())
		return;

	worldRenderPass.setShaderTechnique(m_shader);
	worldRenderPass.setShaderCombination(m_shader);

	render::IProgram* program = m_shader->getCurrentProgram();
	if (!program)
		return;

	Matrix44 viewInverse = worldRenderView.getView().inverse();

	Vector4 cameraPosition = worldRenderView.getEyePosition();
	Matrix44 oceanWorld = translate(-cameraPosition.x(), -m_altitude, -cameraPosition.z());

	render::SimpleRenderBlock* renderBlock = renderContext->alloc< render::SimpleRenderBlock >();

	renderBlock->distance = 0.0f;
	renderBlock->program = program;
	renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
	renderBlock->indexBuffer = m_indexBuffer;
	renderBlock->vertexBuffer = m_vertexBuffer;
	renderBlock->primitives = &m_primitives;

	renderBlock->programParams->beginParameters(renderContext);
	
	worldRenderPass.setProgramParameters(
		renderBlock->programParams,
		oceanWorld,
		Aabb3()
	);

	renderBlock->programParams->setFloatParameter(L"ViewPlane", worldRenderView.getViewFrustum().getNearZ());
	renderBlock->programParams->setFloatParameter(L"OceanRadius", worldRenderView.getViewFrustum().getFarZ());
	renderBlock->programParams->setFloatParameter(L"OceanAltitude", m_altitude);
	renderBlock->programParams->setVectorParameter(L"CameraPosition", cameraPosition);
	renderBlock->programParams->setVectorArrayParameter(L"WaveData", m_waveData, MaxWaves);
	renderBlock->programParams->setMatrixParameter(L"ViewInverse", viewInverse);
	renderBlock->programParams->setFloatParameter(L"ViewRatio", worldRenderView.getViewSize().x / worldRenderView.getViewSize().y);

	if (m_heightfield.validate())
	{
		renderBlock->programParams->setVectorParameter(L"WorldOrigin", -(m_heightfield->getResource().getWorldExtent() * Scalar(0.5f)).xyz1());
		renderBlock->programParams->setVectorParameter(L"WorldExtent", m_heightfield->getResource().getWorldExtent().xyz0());
		//renderBlock->programParams->setTextureParameter(L"Heightfield", m_heightfield->getHeightTexture());
	}

	renderBlock->programParams->endParameters(renderContext);

	renderContext->draw(render::RfAlphaBlend, renderBlock);
}

void OceanEntity::update(const world::EntityUpdate* update)
{
}

	}
}
