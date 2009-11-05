#include <limits>
#include <algorithm>
#include "Terrain/TerrainEntity.h"
#include "Terrain/TerrainEntityData.h"
#include "Terrain/Heightfield.h"
#include "Terrain/TerrainSurfaceCache.h"
#include "World/WorldRenderer.h"
#include "World/WorldRenderView.h"
#include "Resource/IResourceManager.h"
#include "Render/IRenderSystem.h"
#include "Render/VertexElement.h"
#include "Render/VertexBuffer.h"
#include "Render/IndexBuffer.h"
#include "Render/Context/RenderContext.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Half.h"

namespace traktor
{
	namespace terrain
	{
		namespace
		{

struct CullPatch
{
	float distance;
	uint32_t patchId;
	Vector4 patchOrigin;
};

typedef std::pair< float, const TerrainEntity::Patch* > cull_patch_t;

struct PatchFrontToBackPredicate
{
	bool operator () (const CullPatch& lh, const CullPatch& rh) const
	{
		return lh.distance < rh.distance;
	}
};

struct TerrainRenderBlock : public render::SimpleRenderBlock
{
	render::RenderBlock* nested;

	TerrainRenderBlock()
	:	nested(0)
	{
	}

	virtual void render(render::IRenderView* renderView) const
	{
		if (nested)
			nested->render(renderView);

		render::SimpleRenderBlock::render(renderView);
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.TerrainEntity", TerrainEntity, world::Entity)

TerrainEntity::TerrainEntity()
:	m_handleSurface(render::getParameterHandle(L"Surface"))
,	m_handleHeightfield(render::getParameterHandle(L"Heightfield"))
,	m_handleHeightfieldSize(render::getParameterHandle(L"HeightfieldSize"))
,	m_handleNormals(render::getParameterHandle(L"Normals"))
,	m_handleNormalsSize(render::getParameterHandle(L"NormalsSize"))
,	m_handleWorldOrigin(render::getParameterHandle(L"WorldOrigin"))
,	m_handleWorldExtent(render::getParameterHandle(L"WorldExtent"))
,	m_handlePatchOrigin(render::getParameterHandle(L"PatchOrigin"))
,	m_handlePatchExtent(render::getParameterHandle(L"PatchExtent"))
{
}

bool TerrainEntity::create(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem, const TerrainEntityData& data)
{
	m_heightfield = data.m_heightfield;
	if (!resourceManager->bind(m_heightfield))
		return false;

	if (!m_heightfield.validate())
		return false;

	m_shader = data.m_shader;
	if (!resourceManager->bind(m_shader))
		return false;

	uint32_t heightfieldSize = m_heightfield->getResource().getSize();
	T_ASSERT (heightfieldSize > 0);

	const Heightfield::height_t* heights = m_heightfield->getHeights();
	T_ASSERT (heights);

	uint32_t patchDim = m_heightfield->getResource().getPatchDim();
	uint32_t detailSkip = m_heightfield->getResource().getDetailSkip();
	T_ASSERT ((heightfieldSize / detailSkip) % patchDim == 0);

	uint32_t patchCount = heightfieldSize / (patchDim * detailSkip);
	uint32_t patchVertexCount = patchDim * patchDim;

	uint32_t patchTriangleCount = 0;
	for (int lod = 0; lod < 4; ++lod)
	{
		uint32_t lodSkip = 1 << lod;
		uint32_t lodTriangleCount = ((patchDim - 1) / lodSkip) * ((patchDim - 1) / lodSkip) * 2;
		patchTriangleCount += lodTriangleCount;
	}
	uint32_t patchIndexCount = patchTriangleCount * 3;

#if defined(T_USE_TERRAIN_VERTEX_TEXTURE_FETCH)
	std::vector< render::VertexElement > vertexElements;
	vertexElements.push_back(render::VertexElement(render::DuPosition, render::DtHalf2, 0));

	m_vertexBuffer = renderSystem->createVertexBuffer(
		vertexElements,
		patchVertexCount * sizeof(half_t) * 2,
		false
	);
	if (!m_vertexBuffer)
		return 0;

	half_t* vertex = static_cast< half_t* >(m_vertexBuffer->lock());
	T_ASSERT_M (vertex, L"Unable to lock vertex buffer");

	for (uint32_t z = 0; z < patchDim; ++z)
	{
		for (uint32_t x = 0; x < patchDim; ++x)
		{
			*vertex++ = floatToHalf(float(x) / (patchDim - 1));
			*vertex++ = floatToHalf(float(z) / (patchDim - 1));
		}
	}

	m_vertexBuffer->unlock();
#endif

#if !defined(T_USE_TERRAIN_VERTEX_TEXTURE_FETCH)
	std::vector< render::VertexElement > vertexElements;
	vertexElements.push_back(render::VertexElement(render::DuPosition, render::DtFloat3, 0));
#endif

	const Vector4& worldExtent = m_heightfield->getResource().getWorldExtent();

	std::vector< TerrainEntity::Patch > patches;
	for (uint32_t pz = 0; pz < patchCount; ++pz)
	{
		for (uint32_t px = 0; px < patchCount; ++px)
		{
#if !defined(T_USE_TERRAIN_VERTEX_TEXTURE_FETCH)
			Ref< render::VertexBuffer > vertexBuffer = renderSystem->createVertexBuffer(vertexElements, patchVertexCount * sizeof(float) * 3, false);
			if (!vertexBuffer)
				return false;

			float* vertex = static_cast< float* >(vertexBuffer->lock());
			T_ASSERT (vertex);
#endif

			float minHeight =  std::numeric_limits< float >::max();
			float maxHeight = -std::numeric_limits< float >::max();

			for (uint32_t z = 0; z < patchDim; ++z)
			{
				for (uint32_t x = 0; x < patchDim; ++x)
				{
					float fx = float(x) / (patchDim - 1);
					float fz = float(z) / (patchDim - 1);

					uint32_t ix = uint32_t(fx * patchDim * detailSkip) + px * patchDim * detailSkip;
					uint32_t iz = uint32_t(fz * patchDim * detailSkip) + pz * patchDim * detailSkip;

					ix = std::min(ix, heightfieldSize - 1);
					iz = std::min(iz, heightfieldSize - 1);

					float height = float(heights[ix + iz * heightfieldSize]) / 65535.0f;

#if !defined(T_USE_TERRAIN_VERTEX_TEXTURE_FETCH)
					*vertex++ = float(x) / (patchDim - 1);
					*vertex++ = height;
					*vertex++ = float(z) / (patchDim - 1);
#endif

					height = height * worldExtent.y() - worldExtent.y() * 0.5f;

					minHeight = min(minHeight, height);
					maxHeight = max(maxHeight, height);
				}
			}

#if !defined(T_USE_TERRAIN_VERTEX_TEXTURE_FETCH)
			vertexBuffer->unlock();

			TerrainEntity::Patch patch = { minHeight, maxHeight, vertexBuffer };
			m_patches.push_back(patch);
#else
			TerrainEntity::Patch patch = { minHeight, maxHeight };
			m_patches.push_back(patch);
#endif
		}
	}

	std::vector< uint16_t > indices;

	for (uint32_t lod = 0; lod < 4; ++lod)
	{
		uint32_t indexOffset = uint32_t(indices.size());
		uint32_t lodSkip = 1 << lod;

		for (uint32_t y = 0; y < patchDim - 1; y += lodSkip)
		{
			uint32_t offset = y * patchDim;
			for (uint32_t x = 0; x < patchDim - 1; x += lodSkip)
			{
				if (lod > 0 && (x == 0 || y == 0 || x == patchDim - 1 - lodSkip || y == patchDim - 1 - lodSkip))
				{
					int mid = x + offset + (lodSkip >> 1) + (lodSkip >> 1) * patchDim;

					if (x == 0)
					{
						indices.push_back(mid);
						indices.push_back(lodSkip + offset);
						indices.push_back(lodSkip + offset + lodSkip * patchDim);

						for (uint32_t i = 0; i < lodSkip; ++i)
						{
							indices.push_back(mid);
							indices.push_back(offset + i * patchDim + patchDim);
							indices.push_back(offset + i * patchDim);
						}
					}
					else if (x == patchDim - 1 - lodSkip)
					{
						indices.push_back(mid);
						indices.push_back(x + offset + lodSkip * patchDim);
						indices.push_back(x + offset);

						for (uint32_t i = 0; i < lodSkip; ++i)
						{
							indices.push_back(mid);
							indices.push_back(x + offset + i * patchDim + lodSkip);
							indices.push_back(x + offset + i * patchDim + lodSkip + patchDim);
						}
					}
					else
					{
						indices.push_back(mid);
						indices.push_back(x + offset + lodSkip * patchDim);
						indices.push_back(x + offset);

						indices.push_back(mid);
						indices.push_back(x + offset + lodSkip);
						indices.push_back(x + offset + lodSkip + lodSkip * patchDim);
					}

					if (y == 0)
					{
						indices.push_back(mid);
						indices.push_back(x + lodSkip * patchDim + offset + lodSkip);
						indices.push_back(x + lodSkip * patchDim + offset);

						for (uint32_t i = 0; i < lodSkip; ++i)
						{
							indices.push_back(mid);
							indices.push_back(x + offset + i);
							indices.push_back(x + offset + i + 1);
						}
					}
					else if (y == patchDim - 1 - lodSkip)
					{
						indices.push_back(mid);
						indices.push_back(x + offset);
						indices.push_back(x + offset + lodSkip);

						for (uint32_t i = 0; i < lodSkip; ++i)
						{
							indices.push_back(mid);
							indices.push_back(x + offset + i + lodSkip * patchDim + 1);
							indices.push_back(x + offset + i + lodSkip * patchDim);
						}
					}
					else
					{
						indices.push_back(mid);
						indices.push_back(x + offset);
						indices.push_back(x + offset + lodSkip);

						indices.push_back(mid);
						indices.push_back(x + offset + lodSkip * patchDim + lodSkip);
						indices.push_back(x + offset + lodSkip * patchDim);
					}
				}
				else
				{
					indices.push_back(x + offset);
					indices.push_back(lodSkip + x + offset);
					indices.push_back(lodSkip * patchDim + x + offset);

					indices.push_back(lodSkip + x + offset);
					indices.push_back(lodSkip * patchDim + lodSkip + x + offset);
					indices.push_back(lodSkip * patchDim + x + offset);
				}
			}
		}

		m_primitives[lod].setIndexed(
			render::PtTriangles,
			indexOffset,
			uint32_t(indices.size() - indexOffset) / 3,
			0,
			patchVertexCount - 1
		);
	}

	m_indexBuffer = renderSystem->createIndexBuffer(
		render::ItUInt16,
		uint32_t(indices.size() * sizeof(uint16_t)),
		false
	);
	if (!m_indexBuffer)
		return 0;

	uint16_t* index = static_cast< uint16_t* >(m_indexBuffer->lock());
	T_ASSERT_M (index, L"Unable to lock index buffer");

	for (uint32_t i = 0; i < uint32_t(indices.size()); ++i)
		index[i] = indices[i];

	m_indexBuffer->unlock();

	m_surfaceCache = gc_new< TerrainSurfaceCache >();
	if (!m_surfaceCache->create(resourceManager, renderSystem))
		return false;

	m_surface = data.m_surface;
	m_patchCount = patchCount;
	m_patchLodDistance = data.m_patchLodDistance;
	m_surfaceLodDistance = data.m_surfaceLodDistance;

	return true;
}

void TerrainEntity::render(render::RenderContext* renderContext, const world::WorldRenderView* worldRenderView)
{
	Ref< render::ITexture > surface;

	if (!m_heightfield.validate() || !m_shader.validate() || !m_surface)
		return;

	if (!m_shader->hasTechnique(worldRenderView->getTechnique()))
		return;

	bool updateCache =
		bool(worldRenderView->getTechnique() != world::WorldRenderer::getTechniqueShadow()) &&
		bool(worldRenderView->getTechnique() != world::WorldRenderer::getTechniqueVelocity()) &&
		bool(worldRenderView->getTechnique() != world::WorldRenderer::getTechniqueDepth());

	const Vector4& worldExtent = m_heightfield->getResource().getWorldExtent();

	Vector4 patchExtent(worldExtent.x() / float(m_patchCount), worldExtent.y(), worldExtent.z() / float(m_patchCount), 0.0f);
	Scalar patchRadius = patchExtent.length();

	const Vector4& eyePosition = worldRenderView->getEyePosition();
	const Vector4& eyeDirection = worldRenderView->getView().axisZ();

	// Cull patches.
	static AlignedVector< CullPatch > visiblePatches;
	visiblePatches.resize(0);

	Vector4 patchTopLeft = (-worldExtent * Scalar(0.5f)).xyz1();
	Vector4 patchDeltaX = patchExtent * Vector4(1.0f, 0.0f, 0.0f, 0.0f);
	Vector4 patchDeltaZ = patchExtent * Vector4(0.0f, 0.0f, 1.0f, 0.0f);

	for (uint32_t pz = 0; pz < m_patchCount; ++pz)
	{
		Vector4 patchOrigin = patchTopLeft;
		for (uint32_t px = 0; px < m_patchCount; ++px)
		{
			uint32_t patchId = px + pz * m_patchCount;
			const Patch& patch = m_patches[patchId];

			Vector4 patchCenterWorld = patchOrigin + patchExtent * Scalar(0.5f);
			Vector4 patchCenterView = worldRenderView->getView() * patchCenterWorld;

			if (worldRenderView->getCullFrustum().inside(patchCenterView, patchRadius))
			{
				CullPatch cp;

				cp.distance = dot3(patchCenterWorld - eyePosition, eyeDirection);
				cp.patchId = patchId;
				cp.patchOrigin = patchOrigin;

				visiblePatches.push_back(cp);
			}
			else if (updateCache)
			{
				m_surfaceCache->flush(patchId);
			}

			patchOrigin += patchDeltaX;
		}
		patchTopLeft += patchDeltaZ;
	}

	// Sort patches front to back to maximize best use of surface cache and rendering.
	std::sort(visiblePatches.begin(), visiblePatches.end(), PatchFrontToBackPredicate());

	// Render each visible patch.
	for (AlignedVector< CullPatch >::const_iterator i = visiblePatches.begin(); i != visiblePatches.end(); ++i)
	{
		const Patch& patch = m_patches[i->patchId];
		const Vector4& patchOrigin = i->patchOrigin;

		// Calculate which lods to use based one distance to patch center.
		int patchLod = int(i->distance / m_patchLodDistance);
		patchLod = std::max< int >(patchLod, 0);
		patchLod = std::min< int >(patchLod, sizeof_array(m_primitives) - 1);

		int surfaceLod = int(i->distance / m_surfaceLodDistance);
		surfaceLod = std::max< int >(surfaceLod, 0);
		surfaceLod = std::min< int >(surfaceLod, 3);

		TerrainRenderBlock* renderBlock = renderContext->alloc< TerrainRenderBlock >();

		// Update surface cache.
		if (updateCache)
		{
			m_surfaceCache->get(
				worldRenderView,
				renderContext,
				m_surface,
				m_heightfield->getHeightTexture(),
				-worldExtent * Scalar(0.5f),
				worldExtent,
				patchOrigin,
				patchExtent,
				surfaceLod,
				i->patchId,
				// Out
				renderBlock->nested,
				surface
			);
		}

		renderBlock->type = render::RbtOpaque;
		renderBlock->distance = i->distance;
		renderBlock->shader = m_shader;
		renderBlock->shaderParams = renderContext->alloc< render::ShaderParameters >();
		renderBlock->indexBuffer = m_indexBuffer;
#if !defined(T_USE_TERRAIN_VERTEX_TEXTURE_FETCH)
		renderBlock->vertexBuffer = patch.vertexBuffer;
#else
		renderBlock->vertexBuffer = m_vertexBuffer;
#endif
		renderBlock->primitives = &m_primitives[patchLod];

		renderBlock->shaderParams->beginParameters(renderContext);

		renderBlock->shaderParams->setSamplerTexture(m_handleSurface, surface);
		renderBlock->shaderParams->setSamplerTexture(m_handleHeightfield, m_heightfield->getHeightTexture());
		renderBlock->shaderParams->setFloatParameter(m_handleHeightfieldSize, float(m_heightfield->getHeightTexture()->getWidth()));
		renderBlock->shaderParams->setSamplerTexture(m_handleNormals, m_heightfield->getNormalTexture());
		renderBlock->shaderParams->setFloatParameter(m_handleNormalsSize, float(m_heightfield->getNormalTexture()->getWidth()));
		renderBlock->shaderParams->setVectorParameter(m_handleWorldOrigin, -worldExtent * Scalar(0.5f));
		renderBlock->shaderParams->setVectorParameter(m_handleWorldExtent, worldExtent);
		renderBlock->shaderParams->setVectorParameter(m_handlePatchOrigin, patchOrigin);
		renderBlock->shaderParams->setVectorParameter(m_handlePatchExtent, patchExtent);

		worldRenderView->setShaderParameters(renderBlock->shaderParams);

		renderBlock->shaderParams->endParameters(renderContext);

		renderContext->draw(renderBlock);
	}
}

void TerrainEntity::update(const world::EntityUpdate* update)
{
}

	}
}
