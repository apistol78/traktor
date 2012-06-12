#include <algorithm>
#include <limits>
#include "Core/Misc/SafeDestroy.h"
#include "Heightfield/Heightfield.h"
#include "Render/IndexBuffer.h"
#include "Render/IRenderSystem.h"
#include "Render/ISimpleTexture.h"
#include "Render/VertexBuffer.h"
#include "Render/VertexElement.h"
#include "Render/Context/RenderContext.h"
#include "Resource/IResourceManager.h"
#include "Terrain/Terrain.h"
#include "Terrain/TerrainEntity.h"
#include "Terrain/TerrainSurfaceCache.h"
#include "World/IWorldRenderPass.h"
#include "World/WorldRenderView.h"

namespace traktor
{
	namespace terrain
	{
		namespace
		{

const int32_t c_patchLodSteps = 3;
const int32_t c_surfaceLodSteps = 3;

const Vector4 c_lodColor[] =
{
	Vector4(1.0f, 0.0f, 0.0f, 0.0f),
	Vector4(0.0f, 1.0f, 0.0f, 0.0f),
	Vector4(0.0f, 0.0f, 1.0f, 0.0f),
	Vector4(1.0f, 1.0f, 0.0f, 0.0f)
};

struct CullPatch
{
	float distance;
	float area;
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

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.TerrainEntity", TerrainEntity, world::Entity)

TerrainEntity::TerrainEntity(render::IRenderSystem* renderSystem)
:	m_renderSystem(renderSystem)
,	m_visualizeMode(TerrainEntityData::VmDefault)
,	m_handleSurface(render::getParameterHandle(L"Surface"))
,	m_handleSurfaceOffset(render::getParameterHandle(L"SurfaceOffset"))
,	m_handleHeightfield(render::getParameterHandle(L"Heightfield"))
,	m_handleNormals(render::getParameterHandle(L"Normals"))
,	m_handleEye(render::getParameterHandle(L"Eye"))
,	m_handleWorldOrigin(render::getParameterHandle(L"WorldOrigin"))
,	m_handleWorldExtent(render::getParameterHandle(L"WorldExtent"))
,	m_handlePatchOrigin(render::getParameterHandle(L"PatchOrigin"))
,	m_handlePatchExtent(render::getParameterHandle(L"PatchExtent"))
,	m_handlePatchLodColor(render::getParameterHandle(L"PatchLodColor"))
{
}

bool TerrainEntity::create(resource::IResourceManager* resourceManager, const TerrainEntityData& data)
{
	if (!resourceManager->bind(data.getTerrain(), m_terrain))
		return 0;

	m_surfaceCache = new TerrainSurfaceCache();
	if (!m_surfaceCache->create(resourceManager, m_renderSystem))
		return false;

	m_detailSkip = data.getDetailSkip();
	m_patchDim = data.getPatchDim();
	m_patchLodDistance = data.getPatchLodDistance();
	m_patchLodBias = data.getPatchLodBias();
	m_patchLodExponent = data.getPatchLodExponent();
	m_surfaceLodDistance = data.getSurfaceLodDistance();
	m_surfaceLodBias = data.getSurfaceLodBias();
	m_surfaceLodExponent = data.getSurfaceLodExponent();

	if (!createPatches())
		return false;

	m_visualizeMode = data.getVisualizeMode();
	if (m_visualizeMode != TerrainEntityData::VmDefault)
	{
		render::Shader* coarseShader = m_terrain->getTerrainCoarseShader();
		render::Shader* detailShader = m_terrain->getTerrainDetailShader();

		if (coarseShader)
			coarseShader->setCombination(L"VisualizeLods", true);

		if (detailShader)
			detailShader->setCombination(L"VisualizeLods", true);
	}

	return true;
}

void TerrainEntity::render(
	render::RenderContext* renderContext,
	world::WorldRenderView& worldRenderView,
	world::IWorldRenderPass& worldRenderPass
)
{
	if (m_terrain.changed())
	{
		if (!createPatches())
			return;

		m_terrain.consume();
	}

	render::Shader* coarseShader = m_terrain->getTerrainCoarseShader();
	render::Shader* detailShader = m_terrain->getTerrainDetailShader();

	worldRenderPass.setShaderTechnique(coarseShader);
	worldRenderPass.setShaderCombination(coarseShader);

	worldRenderPass.setShaderTechnique(detailShader);
	worldRenderPass.setShaderCombination(detailShader);

	render::IProgram* coarseProgram = coarseShader->getCurrentProgram();
	render::IProgram* detailProgram = detailShader->getCurrentProgram();

	if (!coarseProgram || !detailProgram)
		return;

#if !defined(TARGET_OS_IPHONE)
	// \fixme Assume depth pass enabled; need some information about first pass from camera POV.
	bool updateCache = bool(
		worldRenderPass.getTechnique() == render::getParameterHandle(L"World_DepthWrite") ||
		worldRenderPass.getTechnique() == render::getParameterHandle(L"World_GBufferWrite")
	);
#else
	// Only using a single pass on iOS.
	bool updateCache = true;
#endif

	const Vector4& worldExtent = m_terrain->getHeightfield()->getWorldExtent();

	Matrix44 viewInv = worldRenderView.getView().inverse();
	Vector4 eyePosition = viewInv.translation().xyz1();

	// Cull patches.
	static AlignedVector< CullPatch > visiblePatches;
	visiblePatches.resize(0);

	Vector4 patchExtent(worldExtent.x() / float(m_patchCount), worldExtent.y(), worldExtent.z() / float(m_patchCount), 0.0f);
	Vector4 patchTopLeft = (-worldExtent * Scalar(0.5f)).xyz1();
	Vector4 patchDeltaHalf = patchExtent * Vector4(0.5f, 0.5f, 0.5f, 0.0f);
	Vector4 patchDeltaX = patchExtent * Vector4(1.0f, 0.0f, 0.0f, 0.0f);
	Vector4 patchDeltaZ = patchExtent * Vector4(0.0f, 0.0f, 1.0f, 0.0f);

	// Calculate world frustum.
	Frustum worldCullFrustum = worldRenderView.getCullFrustum();
	for (uint32_t i = 0; i < worldCullFrustum.planes.size(); ++i)
		worldCullFrustum.planes[i] = viewInv * worldCullFrustum.planes[i];

	// Cull patches to world frustum.
	for (uint32_t pz = 0; pz < m_patchCount; ++pz)
	{
		Vector4 patchOrigin = patchTopLeft;
		for (uint32_t px = 0; px < m_patchCount; ++px)
		{
			uint32_t patchId = px + pz * m_patchCount;
			
			const Patch& patch = m_patches[patchId];
			T_ASSERT (patch.minHeight <= patch.maxHeight);

			Vector4 patchCenterWorld = (patchOrigin + patchExtent * Scalar(0.5f)).xyz1();

			Aabb3 patchAabb(
				patchCenterWorld * Vector4(1.0f, 0.0f, 1.0f, 1.0f) - patchDeltaHalf,
				patchCenterWorld * Vector4(1.0f, 0.0f, 1.0f, 1.0f) + patchDeltaHalf
			);

			if (worldCullFrustum.inside(patchAabb) != Frustum::IrOutside)
			{
				Scalar lodDistance = (patchAabb.getCenter() - eyePosition).xyz0().length();

				// Project patch bounding box extents onto view plane and calculate screen area.
				Vector4 extents[8];
				patchAabb.getExtents(extents);

				Vector4 mn(
					std::numeric_limits< float >::max(),
					std::numeric_limits< float >::max(),
					std::numeric_limits< float >::max(),
					std::numeric_limits< float >::max()
				);
				Vector4 mx(
					-std::numeric_limits< float >::max(),
					-std::numeric_limits< float >::max(),
					-std::numeric_limits< float >::max(),
					-std::numeric_limits< float >::max()
				);

				Matrix44 viewProj = worldRenderView.getProjection() * worldRenderView.getView();
				bool clipped = false;

				for (int i = 0; i < sizeof_array(extents); ++i)
				{
					Vector4 p = viewProj * extents[i];
					if (p.w() <= 0.0f)
					{
						clipped = true;
						break;
					}

					// Homogeneous divide.
					p /= p.w();

					// Track screen space extents.
					mn = min(mn, p);
					mx = max(mx, p);
				}

				Vector4 e = mx - mn;

				CullPatch cp;
				cp.distance = lodDistance;
				cp.area = !clipped ? e.x() * e.y() : Scalar(1000.0f);
				cp.patchId = patchId;
				cp.patchOrigin = patchOrigin;
				visiblePatches.push_back(cp);
			}
			else if (updateCache)
			{
				m_patches[patchId].lastPatchLod = c_patchLodSteps;
				m_patches[patchId].lastSurfaceLod = c_surfaceLodSteps;

				m_surfaceCache->flush(patchId);
			}

			patchOrigin += patchDeltaX;
		}
		patchTopLeft += patchDeltaZ;
	}

	// Sort patches front to back to maximize best use of surface cache and rendering.
	std::sort(visiblePatches.begin(), visiblePatches.end(), PatchFrontToBackPredicate());

#if defined(T_USE_TERRAIN_VERTEX_TEXTURE_FETCH)
	static AlignedVector< const CullPatch* > patchLodInstances[LodCount];
	for (uint32_t i = 0; i < LodCount; ++i)
		patchLodInstances[i].resize(0);
#endif

	// Update all patch surfaces.
	if (updateCache)
	{
		m_surfaceCache->begin();
		for (AlignedVector< CullPatch >::const_iterator i = visiblePatches.begin(); i != visiblePatches.end(); ++i)
		{
			Patch& patch = m_patches[i->patchId];
			const Vector4& patchOrigin = i->patchOrigin;

			// Calculate which lods to use based one distance to patch center.
			float patchLodDistance = std::pow(clamp(1.0f - i->area / m_patchLodDistance + m_patchLodBias, 0.0f, 1.0f), m_patchLodExponent);
			float surfaceLodDistance = std::pow(clamp(i->distance / m_surfaceLodDistance + m_surfaceLodBias, 0.0f, 1.0f), m_surfaceLodExponent);

			float patchLodF = patchLodDistance * c_patchLodSteps;
			float surfaceLodF = surfaceLodDistance * c_surfaceLodSteps;

			int32_t patchLod = int32_t(patchLodF + 0.5f);
			int32_t surfaceLod = int32_t(surfaceLodF + 0.5f);

			const float c_lodHysteresisThreshold = 0.5f;
			if (patchLod != patch.lastPatchLod)
			{
				if (std::abs(patchLodF - patch.lastPatchLod) < c_lodHysteresisThreshold)
					patchLod = patch.lastPatchLod;
			}
			if (surfaceLod != patch.lastSurfaceLod)
			{
				if (std::abs(surfaceLodF - patch.lastSurfaceLod) < c_lodHysteresisThreshold)
					surfaceLod = patch.lastSurfaceLod;
			}

			patch.lastPatchLod = patchLod;
			patch.lastSurfaceLod = surfaceLod;

			// Update surface cache.
			render::RenderBlock* renderBlock = 0;
			m_surfaceCache->get(
				renderContext,
				m_terrain,
				-worldExtent * Scalar(0.5f),
				worldExtent,
				patchOrigin,
				patchExtent,
				patch.lastSurfaceLod,
				i->patchId,
				// Out
				renderBlock,
				patch.surfaceOffset
			);

			// Queue render block.
			if (renderBlock)
				renderContext->draw(render::RfOpaque, renderBlock);

			// Queue patch instance.
#if defined(T_USE_TERRAIN_VERTEX_TEXTURE_FETCH)
			patchLodInstances[patchLod].push_back(&(*i));
#endif
		}
	}
#if defined(T_USE_TERRAIN_VERTEX_TEXTURE_FETCH)
	else
	{
		for (AlignedVector< CullPatch >::const_iterator i = visiblePatches.begin(); i != visiblePatches.end(); ++i)
		{
			Patch& patch = m_patches[i->patchId];
			patchLodInstances[patch.lastPatchLod].push_back(&(*i));
		}
	}
#endif

#if defined(T_USE_TERRAIN_VERTEX_TEXTURE_FETCH)

	for (AlignedVector< CullPatch >::const_iterator i = visiblePatches.begin(); i != visiblePatches.end(); ++i)
	{
		Patch& patch = m_patches[i->patchId];

		render::SimpleRenderBlock* renderBlock = renderContext->alloc< render::SimpleRenderBlock >("Terrain patch");

		renderBlock->distance = i->distance;
		renderBlock->program = (patch.lastSurfaceLod == 0) ? detailProgram : coarseProgram;
		renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
		renderBlock->indexBuffer = m_indexBuffer;
		renderBlock->vertexBuffer = m_vertexBuffer;
		renderBlock->primitives = &m_primitives[patch.lastPatchLod];

		renderBlock->programParams->beginParameters(renderContext);
		worldRenderPass.setProgramParameters(renderBlock->programParams, true);

		renderBlock->programParams->setTextureParameter(m_handleSurface, m_surfaceCache->getVirtualTexture());
		renderBlock->programParams->setTextureParameter(m_handleHeightfield, m_terrain->getHeightMap());
		renderBlock->programParams->setTextureParameter(m_handleNormals, m_terrain->getNormalMap());
		renderBlock->programParams->setVectorParameter(m_handleEye, eyePosition);
		renderBlock->programParams->setVectorParameter(m_handleWorldOrigin, -worldExtent * Scalar(0.5f));
		renderBlock->programParams->setVectorParameter(m_handleWorldExtent, worldExtent);
		renderBlock->programParams->setVectorParameter(m_handlePatchExtent, patchExtent);
		renderBlock->programParams->setVectorParameter(m_handleSurfaceOffset, patch.surfaceOffset);
		renderBlock->programParams->setVectorParameter(m_handlePatchOrigin, i->patchOrigin);

		if (m_visualizeMode == TerrainEntityData::VmSurfaceLod)
			renderBlock->programParams->setVectorParameter(m_handlePatchLodColor, c_lodColor[patch.lastSurfaceLod]);
		else if (m_visualizeMode == TerrainEntityData::VmPatchLod)
			renderBlock->programParams->setVectorParameter(m_handlePatchLodColor, c_lodColor[patch.lastPatchLod]);

		renderBlock->programParams->endParameters(renderContext);

		renderContext->draw(render::RfOpaque, renderBlock);
	}

#else

	// Render each visible patch.
	for (AlignedVector< CullPatch >::const_iterator i = visiblePatches.begin(); i != visiblePatches.end(); ++i)
	{
		Patch& patch = m_patches[i->patchId];
		const Vector4& patchOrigin = i->patchOrigin;

		render::SimpleRenderBlock* renderBlock = renderContext->alloc< render::SimpleRenderBlock >("Terrain patch");

		renderBlock->distance = i->distance;
		renderBlock->program = program;
		renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
		renderBlock->indexBuffer = m_indexBuffer;
		renderBlock->vertexBuffer = patch.vertexBuffer;
		renderBlock->primitives = &m_primitives[patch.lastPatchLod];

		renderBlock->programParams->beginParameters(renderContext);
		worldRenderPass.setProgramParameters(renderBlock->programParams, true);
		renderBlock->programParams->setTextureParameter(m_handleSurface, m_surfaceCache->getVirtualTexture());
		renderBlock->programParams->setVectorParameter(m_handleSurfaceOffset, patch.surfaceOffset);
		renderBlock->programParams->setTextureParameter(m_handleHeightfield, m_terrain->getHeightMap());
		renderBlock->programParams->setTextureParameter(m_handleNormals, m_terrain->getNormalMap());
		renderBlock->programParams->setVectorParameter(m_handleEye, eyePosition);
		renderBlock->programParams->setVectorParameter(m_handleWorldOrigin, -worldExtent * Scalar(0.5f));
		renderBlock->programParams->setVectorParameter(m_handleWorldExtent, worldExtent);
		renderBlock->programParams->setVectorParameter(m_handlePatchOrigin, patchOrigin);
		renderBlock->programParams->setVectorParameter(m_handlePatchExtent, patchExtent);

		if (m_visualizeMode == TerrainEntityData::VmSurfaceLod)
			renderBlock->programParams->setVectorParameter(m_handlePatchLodColor, c_lodColor[patch.lastSurfaceLod]);
		else if (m_visualizeMode == TerrainEntityData::VmPatchLod)
			renderBlock->programParams->setVectorParameter(m_handlePatchLodColor, c_lodColor[patch.lastPatchLod]);

		renderBlock->programParams->endParameters(renderContext);

		renderContext->draw(render::RfOpaque, renderBlock);
	}

#endif
}

Aabb3 TerrainEntity::getBoundingBox() const
{
	const Vector4& worldExtent = m_terrain->getHeightfield()->getWorldExtent();
	return Aabb3(-worldExtent, worldExtent);
}

void TerrainEntity::update(const UpdateParams& update)
{
}

bool TerrainEntity::updatePatches(int32_t minX, int32_t minZ, int32_t maxX, int32_t maxZ)
{
	const Vector4& worldExtent = m_terrain->getHeightfield()->getWorldExtent();

	int32_t heightfieldSize = int32_t(m_terrain->getHeightfield()->getSize());

	for (uint32_t pz = 0; pz < m_patchCount; ++pz)
	{
		for (uint32_t px = 0; px < m_patchCount; ++px)
		{
			int32_t pminX = px * m_patchDim * m_detailSkip;
			int32_t pminZ = pz * m_patchDim * m_detailSkip;
			int32_t pmaxX = (px + 1) * m_patchDim * m_detailSkip;
			int32_t pmaxZ = (pz + 1) * m_patchDim * m_detailSkip;

			if (pmaxX < minX || pmaxZ < minZ || pminX > maxX || pminZ > maxZ)
				continue;

			Patch& patch = m_patches[px + pz * m_patchCount];

#if !defined(T_USE_TERRAIN_VERTEX_TEXTURE_FETCH)
			float* vertex = static_cast< float* >(patch.vertexBuffer->lock());
			T_ASSERT (vertex);
#endif

			float minHeight =  std::numeric_limits< float >::max();
			float maxHeight = -std::numeric_limits< float >::max();

			for (uint32_t z = 0; z < m_patchDim; ++z)
			{
				for (uint32_t x = 0; x < m_patchDim; ++x)
				{
					float fx = float(x) / (m_patchDim - 1);
					float fz = float(z) / (m_patchDim - 1);

					int32_t ix = int32_t(fx * m_patchDim * m_detailSkip) + pminX;
					int32_t iz = int32_t(fz * m_patchDim * m_detailSkip) + pminZ;

					ix = std::min(ix, heightfieldSize - 1);
					iz = std::min(iz, heightfieldSize - 1);

					float height = m_terrain->getHeightfield()->getGridHeightNearest(ix, iz);

#if !defined(T_USE_TERRAIN_VERTEX_TEXTURE_FETCH)
					*vertex++ = float(x) / (m_patchDim - 1);
					*vertex++ = height;
					*vertex++ = float(z) / (m_patchDim - 1);
#endif

					height = m_terrain->getHeightfield()->unitToWorld(height);

					minHeight = min(minHeight, height);
					maxHeight = max(maxHeight, height);
				}
			}

			patch.minHeight = minHeight;
			patch.maxHeight = maxHeight;

#if !defined(T_USE_TERRAIN_VERTEX_TEXTURE_FETCH)
			patch.vertexBuffer->unlock();
#endif
		}
	}

	return true;
}

bool TerrainEntity::createPatches()
{
	m_patches.clear();
	m_patchCount = 0;
	safeDestroy(m_indexBuffer);
#if defined(T_USE_TERRAIN_VERTEX_TEXTURE_FETCH)
	safeDestroy(m_vertexBuffer);
#endif

	uint32_t heightfieldSize = m_terrain->getHeightfield()->getSize();
	T_ASSERT (heightfieldSize > 0);

	uint32_t patchVertexCount = m_patchDim * m_patchDim;
	m_patchCount = heightfieldSize / (m_patchDim * m_detailSkip);

#if defined(T_USE_TERRAIN_VERTEX_TEXTURE_FETCH)
	std::vector< render::VertexElement > vertexElements;
	vertexElements.push_back(render::VertexElement(render::DuPosition, render::DtFloat2, 0));
	uint32_t vertexSize = render::getVertexSize(vertexElements);

	m_vertexBuffer = m_renderSystem->createVertexBuffer(
		vertexElements,
		patchVertexCount * vertexSize,
		false
	);
	if (!m_vertexBuffer)
		return false;

	float* vertex = static_cast< float* >(m_vertexBuffer->lock());
	T_ASSERT_M (vertex, L"Unable to lock vertex buffer");

	for (uint32_t z = 0; z < m_patchDim; ++z)
	{
		for (uint32_t x = 0; x < m_patchDim; ++x)
		{
			*vertex++ = float(x) / (m_patchDim - 1);
			*vertex++ = float(z) / (m_patchDim - 1);
		}
	}

	m_vertexBuffer->unlock();
#else
	std::vector< render::VertexElement > vertexElements;
	vertexElements.push_back(render::VertexElement(render::DuPosition, render::DtFloat3, 0));
	uint32_t vertexSize = render::getVertexSize(vertexElements);
#endif

	m_patches.reserve(m_patchCount * m_patchCount);
	for (uint32_t pz = 0; pz < m_patchCount; ++pz)
	{
		for (uint32_t px = 0; px < m_patchCount; ++px)
		{
#if !defined(T_USE_TERRAIN_VERTEX_TEXTURE_FETCH)
			Ref< render::VertexBuffer > vertexBuffer = m_renderSystem->createVertexBuffer(
				vertexElements,
				patchVertexCount * vertexSize,
				false
			);
			if (!vertexBuffer)
				return false;

			TerrainEntity::Patch patch = { 0.0f, 0.0f, vertexBuffer, c_patchLodSteps, c_surfaceLodSteps };
			m_patches.push_back(patch);
#else
			TerrainEntity::Patch patch = { 0.0f, 0.0f, c_patchLodSteps, c_surfaceLodSteps };
			m_patches.push_back(patch);
#endif
		}
	}

	updatePatches(0, 0, heightfieldSize, heightfieldSize);

	std::vector< uint32_t > indices;
	for (uint32_t lod = 0; lod < LodCount; ++lod)
	{
		uint32_t indexOffset = uint32_t(indices.size());
		uint32_t lodSkip = 1 << lod;

#if defined(T_USE_TERRAIN_VERTEX_TEXTURE_FETCH)

		for (uint32_t y = 0; y < m_patchDim - 1; y += lodSkip)
		{
			uint32_t offset = y * m_patchDim;
			for (uint32_t x = 0; x < m_patchDim - 1; x += lodSkip)
			{
				if (lod > 0 && (x == 0 || y == 0 || x == m_patchDim - 1 - lodSkip || y == m_patchDim - 1 - lodSkip))
				{
					int mid = x + offset + (lodSkip >> 1) + (lodSkip >> 1) * m_patchDim;

					if (x == 0)
					{
						indices.push_back(mid);
						indices.push_back(lodSkip + offset);
						indices.push_back(lodSkip + offset + lodSkip * m_patchDim);

						for (uint32_t i = 0; i < lodSkip; ++i)
						{
							indices.push_back(mid);
							indices.push_back(offset + i * m_patchDim + m_patchDim);
							indices.push_back(offset + i * m_patchDim);
						}
					}
					else if (x == m_patchDim - 1 - lodSkip)
					{
						indices.push_back(mid);
						indices.push_back(x + offset + lodSkip * m_patchDim);
						indices.push_back(x + offset);

						for (uint32_t i = 0; i < lodSkip; ++i)
						{
							indices.push_back(mid);
							indices.push_back(x + offset + i * m_patchDim + lodSkip);
							indices.push_back(x + offset + i * m_patchDim + lodSkip + m_patchDim);
						}
					}
					else
					{
						indices.push_back(mid);
						indices.push_back(x + offset + lodSkip * m_patchDim);
						indices.push_back(x + offset);

						indices.push_back(mid);
						indices.push_back(x + offset + lodSkip);
						indices.push_back(x + offset + lodSkip + lodSkip * m_patchDim);
					}

					if (y == 0)
					{
						indices.push_back(mid);
						indices.push_back(x + lodSkip * m_patchDim + offset + lodSkip);
						indices.push_back(x + lodSkip * m_patchDim + offset);

						for (uint32_t i = 0; i < lodSkip; ++i)
						{
							indices.push_back(mid);
							indices.push_back(x + offset + i);
							indices.push_back(x + offset + i + 1);
						}
					}
					else if (y == m_patchDim - 1 - lodSkip)
					{
						indices.push_back(mid);
						indices.push_back(x + offset);
						indices.push_back(x + offset + lodSkip);

						for (uint32_t i = 0; i < lodSkip; ++i)
						{
							indices.push_back(mid);
							indices.push_back(x + offset + i + lodSkip * m_patchDim + 1);
							indices.push_back(x + offset + i + lodSkip * m_patchDim);
						}
					}
					else
					{
						indices.push_back(mid);
						indices.push_back(x + offset);
						indices.push_back(x + offset + lodSkip);

						indices.push_back(mid);
						indices.push_back(x + offset + lodSkip * m_patchDim + lodSkip);
						indices.push_back(x + offset + lodSkip * m_patchDim);
					}
				}
				else
				{
					indices.push_back(x + offset);
					indices.push_back(lodSkip + x + offset);
					indices.push_back(lodSkip * m_patchDim + x + offset);

					indices.push_back(lodSkip + x + offset);
					indices.push_back(lodSkip * m_patchDim + lodSkip + x + offset);
					indices.push_back(lodSkip * m_patchDim + x + offset);
				}
			}
		}

		uint32_t indexEndOffset = uint32_t(indices.size());
		T_ASSERT ((indexEndOffset - indexOffset) % 3 == 0);

		uint32_t minIndex = *std::min_element(indices.begin() + indexOffset, indices.begin() + indexEndOffset);
		uint32_t maxIndex = *std::max_element(indices.begin() + indexOffset, indices.begin() + indexEndOffset);

		T_ASSERT (minIndex < patchVertexCount);
		T_ASSERT (maxIndex < patchVertexCount);
		//T_ASSERT (maxIndex + patchVertexCount * (PatchInstanceCount - 1) < 65536);

		m_primitives[lod].setIndexed(
			render::PtTriangles,
			indexOffset,
			(indexEndOffset - indexOffset) / 3,
			minIndex,
			maxIndex
		);
#else

		for (uint32_t y = 0; y < m_patchDim - 1; y += lodSkip)
		{
			uint32_t offset = y * m_patchDim;
			for (uint32_t x = 0; x < m_patchDim - 1; x += lodSkip)
			{
				if (lod > 0 && (x == 0 || y == 0 || x == m_patchDim - 1 - lodSkip || y == m_patchDim - 1 - lodSkip))
				{
					int mid = x + offset + (lodSkip >> 1) + (lodSkip >> 1) * m_patchDim;

					if (x == 0)
					{
						indices.push_back(mid);
						indices.push_back(lodSkip + offset);
						indices.push_back(lodSkip + offset + lodSkip * m_patchDim);

						for (uint32_t i = 0; i < lodSkip; ++i)
						{
							indices.push_back(mid);
							indices.push_back(offset + i * m_patchDim + m_patchDim);
							indices.push_back(offset + i * m_patchDim);
						}
					}
					else if (x == m_patchDim - 1 - lodSkip)
					{
						indices.push_back(mid);
						indices.push_back(x + offset + lodSkip * m_patchDim);
						indices.push_back(x + offset);

						for (uint32_t i = 0; i < lodSkip; ++i)
						{
							indices.push_back(mid);
							indices.push_back(x + offset + i * m_patchDim + lodSkip);
							indices.push_back(x + offset + i * m_patchDim + lodSkip + m_patchDim);
						}
					}
					else
					{
						indices.push_back(mid);
						indices.push_back(x + offset + lodSkip * m_patchDim);
						indices.push_back(x + offset);

						indices.push_back(mid);
						indices.push_back(x + offset + lodSkip);
						indices.push_back(x + offset + lodSkip + lodSkip * m_patchDim);
					}

					if (y == 0)
					{
						indices.push_back(mid);
						indices.push_back(x + lodSkip * m_patchDim + offset + lodSkip);
						indices.push_back(x + lodSkip * m_patchDim + offset);

						for (uint32_t i = 0; i < lodSkip; ++i)
						{
							indices.push_back(mid);
							indices.push_back(x + offset + i);
							indices.push_back(x + offset + i + 1);
						}
					}
					else if (y == m_patchDim - 1 - lodSkip)
					{
						indices.push_back(mid);
						indices.push_back(x + offset);
						indices.push_back(x + offset + lodSkip);

						for (uint32_t i = 0; i < lodSkip; ++i)
						{
							indices.push_back(mid);
							indices.push_back(x + offset + i + lodSkip * m_patchDim + 1);
							indices.push_back(x + offset + i + lodSkip * m_patchDim);
						}
					}
					else
					{
						indices.push_back(mid);
						indices.push_back(x + offset);
						indices.push_back(x + offset + lodSkip);

						indices.push_back(mid);
						indices.push_back(x + offset + lodSkip * m_patchDim + lodSkip);
						indices.push_back(x + offset + lodSkip * m_patchDim);
					}
				}
				else
				{
					indices.push_back(x + offset);
					indices.push_back(lodSkip + x + offset);
					indices.push_back(lodSkip * m_patchDim + x + offset);

					indices.push_back(lodSkip + x + offset);
					indices.push_back(lodSkip * m_patchDim + lodSkip + x + offset);
					indices.push_back(lodSkip * m_patchDim + x + offset);
				}
			}
		}

		uint32_t indexEndOffset = uint32_t(indices.size());

		uint32_t minIndex = *std::min_element(indices.begin() + indexOffset, indices.begin() + indexEndOffset);
		uint32_t maxIndex = *std::max_element(indices.begin() + indexOffset, indices.begin() + indexEndOffset);

		m_primitives[lod].setIndexed(
			render::PtTriangles,
			indexOffset,
			(indexEndOffset - indexOffset) / 3,
			minIndex,
			maxIndex
		);

#endif
	}

	m_indexBuffer = m_renderSystem->createIndexBuffer(
		render::ItUInt32,
		uint32_t(indices.size() * sizeof(uint32_t)),
		false
	);
	if (!m_indexBuffer)
		return false;

	uint32_t* index = static_cast< uint32_t* >(m_indexBuffer->lock());
	T_ASSERT_M (index, L"Unable to lock index buffer");

	for (uint32_t i = 0; i < uint32_t(indices.size()); ++i)
		index[i] = indices[i];

	m_indexBuffer->unlock();
	return true;
}

	}
}
