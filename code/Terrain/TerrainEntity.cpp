#include <algorithm>
#include <limits>
#include "Core/Containers/AlignedVector.h"
#include "Core/Log/Log.h"
#include "Core/Math/Half.h"
#include "Core/Math/Log2.h"
#include "Core/Misc/AutoPtr.h"
#include "Core/Misc/SafeDestroy.h"
#include "Heightfield/Heightfield.h"
#include "Heightfield/MaterialMask.h"
#include "Render/IndexBuffer.h"
#include "Render/IRenderSystem.h"
#include "Render/ISimpleTexture.h"
#include "Render/VertexBuffer.h"
#include "Render/VertexElement.h"
#include "Render/Context/RenderContext.h"
#include "Resource/IResourceManager.h"
#include "Terrain/TerrainEntity.h"
#include "Terrain/TerrainSurface.h"
#include "Terrain/TerrainSurfaceCache.h"
#include "World/IWorldRenderPass.h"
#include "World/WorldRenderView.h"

namespace traktor
{
	namespace terrain
	{
		namespace
		{

const Guid c_guidTerrainShaderVFetch(L"{480B7C64-5494-A74A-8485-F2CA15A900E6}");
const Guid c_guidTerrainShaderStatic(L"{557537A0-F1E3-AF4C-ACB9-FD862FC3265C}");

#if defined(T_USE_TERRAIN_VERTEX_TEXTURE_FETCH)
const uint32_t c_skipHeightTexture = 1;
#else
const uint32_t c_skipHeightTexture = 4;
#endif
const uint32_t c_skipNormalTexture = 2;
const uint32_t c_skipMaterialMaskTexture = 1;
const uint32_t c_skipHeightTextureEditor = 4;
const uint32_t c_skipNormalTextureEditor = 2;
const uint32_t c_skipMaterialMaskTextureEditor = 1;
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

TerrainEntity::TerrainEntity(render::IRenderSystem* renderSystem, bool editorMode)
:	m_renderSystem(renderSystem)
,	m_editorMode(editorMode)
,	m_visualizeMode(TerrainEntityData::VmDefault)
,	m_handleSurface(render::getParameterHandle(L"Surface"))
,	m_handleSurfaceOffset(render::getParameterHandle(L"SurfaceOffset"))
,	m_handleHeightfield(render::getParameterHandle(L"Heightfield"))
,	m_handleHeightfieldSize(render::getParameterHandle(L"HeightfieldSize"))
,	m_handleNormals(render::getParameterHandle(L"Normals"))
,	m_handleNormalsSize(render::getParameterHandle(L"NormalsSize"))
,	m_handleWorldOrigin(render::getParameterHandle(L"WorldOrigin"))
,	m_handleWorldExtent(render::getParameterHandle(L"WorldExtent"))
,	m_handlePatchOrigin(render::getParameterHandle(L"PatchOrigin"))
,	m_handlePatchExtent(render::getParameterHandle(L"PatchExtent"))
,	m_handlePatchLodColor(render::getParameterHandle(L"PatchLodColor"))
{
}

bool TerrainEntity::create(resource::IResourceManager* resourceManager, const TerrainEntityData& data)
{
	m_heightfield = data.getHeightfield();
	if (!resourceManager->bind(m_heightfield))
		return false;

	if (!m_heightfield.validate())
		return false;

	m_materialMask = data.getMaterialMask();
	if (!resourceManager->bind(m_materialMask))
		return false;

	if (!m_materialMask.validate())
		return false;

#if defined(T_USE_TERRAIN_VERTEX_TEXTURE_FETCH)
	m_shader = c_guidTerrainShaderVFetch;
#else
	m_shader = c_guidTerrainShaderStatic;
#endif
	if (!resourceManager->bind(m_shader))
		return false;

	if (!createPatches())
		return false;

	if (!createTextures())
		return false;

	m_surfaceCache = new TerrainSurfaceCache();
	if (!m_surfaceCache->create(resourceManager, m_renderSystem))
		return false;

	m_surface = data.getSurface();
	m_patchLodDistance = data.getPatchLodDistance();
	m_patchLodBias = data.getPatchLodBias();
	m_patchLodExponent = data.getPatchLodExponent();
	m_surfaceLodDistance = data.getSurfaceLodDistance();
	m_surfaceLodBias = data.getSurfaceLodBias();
	m_surfaceLodExponent = data.getSurfaceLodExponent();

	if (m_surface)
	{
		std::vector< resource::Proxy< render::Shader > >& layers = m_surface->getLayers();
		for (std::vector< resource::Proxy< render::Shader > >::iterator i = layers.begin(); i != layers.end(); ++i)
		{
			if (!resourceManager->bind(*i))
				return false;
		}
	}

	m_visualizeMode = data.getVisualizeMode();
	if (m_visualizeMode != TerrainEntityData::VmDefault)
		m_shader->setCombination(L"VisualizeLods", true);
	else
		m_shader->setCombination(L"VisualizeLods", false);

	return true;
}

void TerrainEntity::render(
	render::RenderContext* renderContext,
	world::WorldRenderView& worldRenderView,
	world::IWorldRenderPass& worldRenderPass
)
{
	if (!m_heightfield.valid() || !m_materialMask.valid())
	{
		if (!m_heightfield.validate() || !m_materialMask.validate())
			return;

		if (!createPatches())
			return;

		if (!createTextures())
			return;
	}

	if (!m_shader.validate() || !m_surface)
		return;

	worldRenderPass.setShaderTechnique(m_shader);
	worldRenderPass.setShaderCombination(m_shader);

	render::IProgram* program = m_shader->getCurrentProgram();
	if (!program)
		return;

	bool updateCache =
		bool(worldRenderPass.getTechnique() == render::getParameterHandle(L"World_ForwardColor")) ||
		bool(worldRenderPass.getTechnique() == render::getParameterHandle(L"World_PreLitColor"));

	const Vector4& worldExtent = m_heightfield->getResource().getWorldExtent();
	Vector4 patchExtent(worldExtent.x() / float(m_patchCount), worldExtent.y(), worldExtent.z() / float(m_patchCount), 0.0f);

	const Vector4& eyePosition = worldRenderView.getEyePosition();

	// Cull patches.
	static AlignedVector< CullPatch > visiblePatches;
	visiblePatches.resize(0);

	Vector4 patchTopLeft = (-worldExtent * Scalar(0.5f)).xyz1();
	Vector4 patchDeltaHalf = patchExtent * Vector4(0.5f, 0.0f, 0.5f, 0.0f);
	Vector4 patchDeltaX = patchExtent * Vector4(1.0f, 0.0f, 0.0f, 0.0f);
	Vector4 patchDeltaZ = patchExtent * Vector4(0.0f, 0.0f, 1.0f, 0.0f);

	Matrix44 viewInv = worldRenderView.getView().inverseOrtho();

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

			Vector4 patchCenterWorld = (patchOrigin + patchExtent * Scalar(0.5f)).xyz1();

			Aabb3 patchAabb(
				patchCenterWorld - patchDeltaHalf + Vector4(0.0f, patch.minHeight, 0.0f, 0.0f),
				patchCenterWorld + patchDeltaHalf + Vector4(0.0f, patch.maxHeight, 0.0f, 0.0f)
			);

			if (worldCullFrustum.inside(patchAabb) != Frustum::IrOutside)
			{
				Scalar lodDistance = (patchCenterWorld - eyePosition).xyz0().length();

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
				m_surface,
				m_heightTexture,
				m_materialMaskTexture,
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
		}
	}

#if defined(T_USE_TERRAIN_VERTEX_TEXTURE_FETCH)

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
		renderBlock->vertexBuffer = m_vertexBuffer;
		renderBlock->primitives = &m_primitives[patch.lastPatchLod];

		renderBlock->programParams->beginParameters(renderContext);
		worldRenderPass.setProgramParameters(renderBlock->programParams);
		renderBlock->programParams->setTextureParameter(m_handleSurface, m_surfaceCache->getVirtualTexture());
		renderBlock->programParams->setVectorParameter(m_handleSurfaceOffset, patch.surfaceOffset);
		renderBlock->programParams->setTextureParameter(m_handleHeightfield, m_heightTexture);
		renderBlock->programParams->setFloatParameter(m_handleHeightfieldSize, float(m_heightTexture->getWidth()));
		renderBlock->programParams->setTextureParameter(m_handleNormals, m_normalTexture);
		renderBlock->programParams->setFloatParameter(m_handleNormalsSize, float(m_normalTexture->getWidth()));
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
		worldRenderPass.setProgramParameters(renderBlock->programParams);
		renderBlock->programParams->setTextureParameter(m_handleSurface, m_surfaceCache->getVirtualTexture());
		renderBlock->programParams->setVectorParameter(m_handleSurfaceOffset, patch.surfaceOffset);
		renderBlock->programParams->setTextureParameter(m_handleHeightfield, m_heightTexture);
		renderBlock->programParams->setFloatParameter(m_handleHeightfieldSize, float(m_heightTexture->getWidth()));
		renderBlock->programParams->setTextureParameter(m_handleNormals, m_normalTexture);
		renderBlock->programParams->setFloatParameter(m_handleNormalsSize, float(m_normalTexture->getWidth()));
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

void TerrainEntity::update(const world::EntityUpdate* update)
{
}

bool TerrainEntity::updatePatches(int32_t minX, int32_t minZ, int32_t maxX, int32_t maxZ)
{
	const Vector4& worldExtent = m_heightfield->getResource().getWorldExtent();

	const hf::height_t* heights = m_heightfield->getHeights();
	T_ASSERT (heights);

	uint32_t heightfieldSize = m_heightfield->getResource().getSize();
	T_ASSERT (heightfieldSize > 0);

	uint32_t patchDim = m_heightfield->getResource().getPatchDim();
	uint32_t detailSkip = m_heightfield->getResource().getDetailSkip();
	T_ASSERT ((heightfieldSize / detailSkip) % patchDim == 0);

	for (uint32_t pz = 0; pz < m_patchCount; ++pz)
	{
		for (uint32_t px = 0; px < m_patchCount; ++px)
		{
			int32_t pminX = px * patchDim * detailSkip;
			int32_t pminZ = pz * patchDim * detailSkip;
			int32_t pmaxX = (px + 1) * patchDim * detailSkip;
			int32_t pmaxZ = (pz + 1) * patchDim * detailSkip;

			if (pmaxX < minX || pmaxZ < minZ || pminX > maxX || pminZ > maxZ)
				continue;

			Patch& patch = m_patches[px + pz * m_patchCount];

#if !defined(T_USE_TERRAIN_VERTEX_TEXTURE_FETCH)
			float* vertex = static_cast< float* >(patch.vertexBuffer->lock());
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

					uint32_t ix = uint32_t(fx * patchDim * detailSkip) + pminX;
					uint32_t iz = uint32_t(fz * patchDim * detailSkip) + pminZ;

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

	uint32_t heightfieldSize = m_heightfield->getResource().getSize();
	T_ASSERT (heightfieldSize > 0);

	const hf::height_t* heights = m_heightfield->getHeights();
	T_ASSERT (heights);

	uint32_t patchDim = m_heightfield->getResource().getPatchDim();
	uint32_t detailSkip = m_heightfield->getResource().getDetailSkip();
	T_ASSERT ((heightfieldSize / detailSkip) % patchDim == 0);
	uint32_t patchVertexCount = patchDim * patchDim;

	m_patchCount = heightfieldSize / (patchDim * detailSkip);

#if defined(T_USE_TERRAIN_VERTEX_TEXTURE_FETCH)
	std::vector< render::VertexElement > vertexElements;
	vertexElements.push_back(render::VertexElement(render::DuPosition, render::DtHalf2, 0));

	m_vertexBuffer = m_renderSystem->createVertexBuffer(
		vertexElements,
		patchVertexCount * sizeof(half_t) * 2,
		false
	);
	if (!m_vertexBuffer)
		return false;

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

	m_patches.reserve(m_patchCount * m_patchCount);
	for (uint32_t pz = 0; pz < m_patchCount; ++pz)
	{
		for (uint32_t px = 0; px < m_patchCount; ++px)
		{
#if !defined(T_USE_TERRAIN_VERTEX_TEXTURE_FETCH)
			// Create dynamic vertex buffers if we're in editor mode.
			Ref< render::VertexBuffer > vertexBuffer = m_renderSystem->createVertexBuffer(
				vertexElements,
				patchVertexCount * sizeof(float) * 3,
				m_editorMode
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
	}

	m_indexBuffer = m_renderSystem->createIndexBuffer(
		render::ItUInt16,
		uint32_t(indices.size() * sizeof(uint16_t)),
		false
	);
	if (!m_indexBuffer)
		return false;

	uint16_t* index = static_cast< uint16_t* >(m_indexBuffer->lock());
	T_ASSERT_M (index, L"Unable to lock index buffer");

	for (uint32_t i = 0; i < uint32_t(indices.size()); ++i)
		index[i] = indices[i];

	m_indexBuffer->unlock();
	return true;
}

bool TerrainEntity::updateTextures(bool normals, bool heights, bool materials)
{
	const float c_scaleHeight = 20.0f;

	if (normals)
	{
		const float c_directions[] =
		{
			0.0f, 0.0f,
			-1.0f, -1.0f,
			 1.0f, -1.0f,
			 1.0f,  1.0f,
			-1.0f,  1.0f
		};

		const uint32_t c_pattern[] =
		{
			0, 1, 2,
			0, 2, 3,
			0, 3, 4,
			0, 4, 1
		};

		render::ITexture::Lock lock;
		if (!m_normalTexture->lock(0, lock))
			return false;

		uint8_t* np = static_cast< uint8_t* >(lock.bits);

		const Vector4& worldExtent = m_heightfield->getResource().getWorldExtent();
		float scaleHeight = worldExtent.y() * c_scaleHeight;

		int32_t size = m_heightfield->getResource().getSize();
		int32_t dim = m_normalTexture->getWidth();

		for (int32_t v = 0; v < dim; ++v)
		{
			float gz = float(v * size) / dim;
			for (int32_t u = 0; u < dim; ++u)
			{
				float gx = float(u * size) / dim;

				float h[] =
				{
					m_heightfield->getGridHeight(gx, gz) * scaleHeight,
					m_heightfield->getGridHeight(gx - 1, gz - 1) * scaleHeight,
					m_heightfield->getGridHeight(gx + 1, gz - 1) * scaleHeight,
					m_heightfield->getGridHeight(gx + 1, gz + 1) * scaleHeight,
					m_heightfield->getGridHeight(gx - 1, gz + 1) * scaleHeight
				};

				Vector4 normal = Vector4::zero();
				for (uint32_t i = 0; i < 4; ++i)
				{
					const uint32_t* p = &c_pattern[i * 3];

					Vector4 p1(c_directions[p[0] * 2 + 0], h[p[0]], c_directions[p[0] * 2 + 1], 0.0f);
					Vector4 p2(c_directions[p[1] * 2 + 0], h[p[1]], c_directions[p[1] * 2 + 1], 0.0f);
					Vector4 p3(c_directions[p[2] * 2 + 0], h[p[2]], c_directions[p[2] * 2 + 1], 0.0f);

					normal += cross(p3 - p1, p2 - p1);
				}

				normal = normal.normalized();
				normal = normal * Vector4(0.5f, 0.5f, 0.5f, 0.0f) + Vector4(0.5f, 0.5f, 0.5f, 0.0f);

				uint8_t* p = &np[(u + v * dim) * 4];
				p[0] = uint8_t(normal.x() * 255);
				p[1] = uint8_t(normal.y() * 255);
				p[2] = uint8_t(normal.z() * 255);
				p[3] = 0;
			}
		}

		m_normalTexture->unlock(0);
	}

	if (heights)
	{
		render::ITexture::Lock lock;
		if (!m_heightTexture->lock(0, lock))
			return false;

		half_t* hp = static_cast< half_t* >(lock.bits);

		int32_t size = m_heightfield->getResource().getSize();
		int32_t dim = m_heightTexture->getWidth();

		for (int32_t v = 0; v < dim; ++v)
		{
			float gz = float(v * size) / dim;
			for (int32_t u = 0; u < dim; ++u)
			{
				float gx = float(u * size) / dim;
				float h = m_heightfield->getGridHeight(gx, gz);
				hp[u + v * dim] = floatToHalf(h);
			}
		}

		m_heightTexture->unlock(0);
	}

	if (materials)
	{
		render::ITexture::Lock lock;
		if (!m_materialMaskTexture->lock(0, lock))
			return false;

		uint8_t* mp = static_cast< uint8_t* >(lock.bits);

		int32_t size = m_materialMask->getSize();
		int32_t dim = m_materialMaskTexture->getWidth();

		for (int32_t v = 0; v < dim; ++v)
		{
			int32_t gz = (v * size) / dim;
			for (int32_t u = 0; u < dim; ++u)
			{
				int32_t gx = (u * size) / dim;
				mp[u + v * dim] = m_materialMask->getId(gx, gz);
			}
		}

		m_materialMaskTexture->unlock(0);
	}

	return true;
}

bool TerrainEntity::createTextures()
{
	safeDestroy(m_normalTexture);
	safeDestroy(m_heightTexture);
	safeDestroy(m_materialMaskTexture);

	{
		uint32_t size = m_heightfield->getResource().getSize();
		T_ASSERT (size > 0);

		uint32_t dim = nearestLog2(size);
		dim /= m_editorMode ? c_skipNormalTextureEditor : c_skipNormalTexture;
		T_ASSERT (dim > 0);

		render::SimpleTextureCreateDesc desc;
		desc.width = dim;
		desc.height = dim;
		desc.mipCount = 1;
		desc.format = render::TfR8G8B8A8;
		desc.immutable = false;

		if (!(m_normalTexture = m_renderSystem->createSimpleTexture(desc)))
			return false;
	}

	{
		uint32_t size = m_heightfield->getResource().getSize();
		T_ASSERT (size > 0);

		uint32_t dim = nearestLog2(size);
		dim /= m_editorMode ? c_skipHeightTextureEditor : c_skipHeightTexture;
		T_ASSERT (dim > 0);

		render::SimpleTextureCreateDesc desc;
		desc.width = dim;
		desc.height = dim;
		desc.mipCount = 1;
		desc.format = render::TfR16F;
		desc.immutable = false;

		if (!(m_heightTexture = m_renderSystem->createSimpleTexture(desc)))
			return false;
	}

	{
		uint32_t size = m_materialMask->getSize();
		T_ASSERT (size > 0);

		uint32_t dim = nearestLog2(size);
		dim /= m_editorMode ? c_skipMaterialMaskTextureEditor : c_skipMaterialMaskTexture;
		T_ASSERT (dim > 0);

		render::SimpleTextureCreateDesc desc;
		desc.width = dim;
		desc.height = dim;
		desc.mipCount = 1;
		desc.format = render::TfR8;
		desc.immutable = false;

		if (!(m_materialMaskTexture = m_renderSystem->createSimpleTexture(desc)))
			return false;
	}

	updateTextures(true, true, true);

	return true;
}

	}
}
