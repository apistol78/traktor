/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/Entity/DisplacementWorldComponent.h"

#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Render/Buffer.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderTargetSet.h"
#include "Render/ScreenRenderer.h"
#include "Render/VertexElement.h"
#include "Resource/IResourceManager.h"
#include "World/Entity/DisplacementEntityComponent.h"
#include "World/Entity/DisplacementWorldComponentData.h"
#include "World/WorldHandles.h"
#include "World/WorldRenderView.h"

#include <cmath>

namespace traktor::world
{
namespace
{

const resource::Id< render::Shader > c_reprojectShader(Guid(L"{5A1C7E20-3B4D-4A61-9F2E-11C0D8A45E10}"));
const resource::Id< render::Shader > c_rasterizeVolumesShader(Guid(L"{5A1C7E20-3B4D-4A61-9F2E-11C0D8A45E11}"));

const uint32_t c_maxVolumeCount = 256;

// Permutations
const render::Handle s_handleWorld_DisplacementEnable(L"World_DisplacementEnable");

// Parameters
const render::Handle s_handleDisplacement_PreviousMask(L"Displacement_PreviousMask");
const render::Handle s_handleDisplacement_Reproject(L"Displacement_Reproject");
const render::Handle s_handleDisplacement_MaskExtent(L"Displacement_MaskExtent");
const render::Handle s_handleDisplacement_Volumes(L"Displacement_Volumes");
const render::Handle s_handleWorld_DisplacementMap(L"World_DisplacementMap");
const render::Handle s_handleWorld_DisplacementExtent(L"World_DisplacementExtent");

#pragma pack(1)

struct Vertex
{
	float position[2];
};

#pragma pack()

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.DisplacementWorldComponent", DisplacementWorldComponent, IWorldComponent)

bool DisplacementWorldComponent::create(
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	const DisplacementWorldComponentData& data)
{
	if (!resourceManager->bind(c_reprojectShader, m_reprojectShader))
		return false;
	if (!resourceManager->bind(c_rasterizeVolumesShader, m_rasterizeVolumesShader))
		return false;

	m_screenRenderer = new render::ScreenRenderer();
	if (!m_screenRenderer->create(renderSystem))
		return false;

	// Double buffered displacement masks.
	{
		render::RenderTargetSetCreateDesc desc = {};
		desc.count = 1;
		desc.width = data.m_resolution;
		desc.height = data.m_resolution;
		desc.multiSample = 0;
		desc.createDepthStencil = false;
		desc.usingPrimaryDepthStencil = false;
		desc.targets[0].format = render::TfR16G16B16A16F;

		for (int32_t i = 0; i < sizeof_array(m_maskTargetSet); ++i)
		{
			m_maskTargetSet[i] = renderSystem->createRenderTargetSet(desc, nullptr, T_FILE_LINE_W);
			if (!m_maskTargetSet[i])
				return false;
		}
	}

	// Unit quad, instanced once per volume and scaled to the volume footprint.
	{
		AlignedVector< render::VertexElement > vertexElements;
		vertexElements.push_back(render::VertexElement(render::DataUsage::Position, render::DtFloat2, offsetof(Vertex, position)));
		T_ASSERT(render::getVertexSize(vertexElements) == sizeof(Vertex));
		m_vertexLayout = renderSystem->createVertexLayout(vertexElements);

		m_vertexBuffer = renderSystem->createBuffer(render::BuVertex, 4 * sizeof(Vertex), false, T_FILE_LINE_W);
		if (!m_vertexBuffer)
			return false;

		Vertex* vertex = static_cast< Vertex* >(m_vertexBuffer->lock());
		if (!vertex)
			return false;

		*vertex++ = { { -1.0f, -1.0f } };
		*vertex++ = { { -1.0f, 1.0f } };
		*vertex++ = { { 1.0f, 1.0f } };
		*vertex++ = { { 1.0f, -1.0f } };

		m_vertexBuffer->unlock();

		m_indexBuffer = renderSystem->createBuffer(render::BuIndex, 3 * 2 * sizeof(uint16_t), false, T_FILE_LINE_W);
		if (!m_indexBuffer)
			return false;

		uint16_t* index = static_cast< uint16_t* >(m_indexBuffer->lock());
		if (!index)
			return false;

		*index++ = 0;
		*index++ = 1;
		*index++ = 2;

		*index++ = 0;
		*index++ = 2;
		*index++ = 3;

		m_indexBuffer->unlock();
	}

	m_volumeBuffer = renderSystem->createBuffer(render::BuStructured, c_maxVolumeCount * sizeof(VolumeData), true, T_FILE_LINE_W);
	if (!m_volumeBuffer)
		return false;

	m_renderSystem = renderSystem;
	m_resolution = data.m_resolution;
	m_extent = data.m_extent;
	m_fadeRate = data.m_fadeRate;
	m_maskExtent = Vector4(0.0f, 0.0f, m_extent, 1.0f / m_extent);
	return true;
}

void DisplacementWorldComponent::destroy()
{
	safeDestroy(m_volumeBuffer);
	safeDestroy(m_indexBuffer);
	safeDestroy(m_vertexBuffer);
	m_vertexLayout = nullptr;
	for (int32_t i = 0; i < sizeof_array(m_maskTargetSet); ++i)
		safeDestroy(m_maskTargetSet[i]);
	safeDestroy(m_screenRenderer);
	m_rasterizeVolumesShader.clear();
	m_reprojectShader.clear();
	m_renderSystem = nullptr;
}

void DisplacementWorldComponent::update(World* world, const UpdateParams& update)
{
}

void DisplacementWorldComponent::setup(
	render::RenderGraph& renderGraph,
	const WorldRenderView& worldRenderView,
	const AlignedVector< Object* >& renderables)
{
	if (worldRenderView.getSnapshot())
		return;

	const float deltaTime = (float)worldRenderView.getDeltaTime();

	const float texelExtent = m_extent / (float)m_resolution;
	const Vector4 eyePosition = worldRenderView.getEyePosition();
	const Vector4 center(
		std::floor((float)eyePosition.x() / texelExtent) * texelExtent,
		0.0f,
		std::floor((float)eyePosition.z() / texelExtent) * texelExtent,
		0.0f);

	const bool haveHistory = m_haveMask;
	const Vector4 centerDelta = center - m_center;

	m_center = center;
	m_maskExtent = Vector4((float)center.x(), (float)center.z(), m_extent, 1.0f / m_extent);
	m_haveMask = true;

	const float reprojectU = (float)centerDelta.x() / m_extent;
	const float reprojectV = (float)centerDelta.z() / m_extent;
	const float fade = std::exp(-m_fadeRate * deltaTime);

	uint32_t volumeCount = 0;
	if (VolumeData* volumeData = (VolumeData*)m_volumeBuffer->lock())
	{
		const float halfExtent = m_extent * 0.5f;
		for (const Object* renderable : renderables)
		{
			const DisplacementEntityComponent* volume = static_cast< const DisplacementEntityComponent* >(renderable);

			if (volumeCount >= c_maxVolumeCount)
				break;

			const float radius = volume->getRadius();
			const float strength = volume->getStrength();
			if (radius <= FUZZY_EPSILON || strength <= FUZZY_EPSILON)
				continue;

			const Vector4 position = volume->getPosition();
			const float dx = (float)position.x() - (float)center.x();
			const float dz = (float)position.z() - (float)center.z();
			if (std::abs(dx) > halfExtent + radius || std::abs(dz) > halfExtent + radius)
				continue;

			VolumeData& vd = volumeData[volumeCount++];
			vd.centerX = (float)position.x();
			vd.centerZ = (float)position.z();
			vd.radius = radius;
			vd.strength = strength;
			vd.falloff = volume->getFalloff();
			vd.press = volume->getPress();
			vd.dummy0 = 0.0f;
			vd.dummy1 = 0.0f;
		}

		m_volumeBuffer->unlock();
	}

	m_current = 1 - m_current;

	render::IRenderTargetSet* maskTargetSet = m_maskTargetSet[m_current];
	render::ITexture* previousMaskTexture = m_maskTargetSet[1 - m_current]->getColorTexture(0);

	const render::RGTargetSet maskTargetSetId = renderGraph.addExplicitTargetSet(L"Displacement mask", maskTargetSet);
	const Vector4 maskExtent = m_maskExtent;

	Ref< render::RenderPass > rp = new render::RenderPass(L"Displacement mask");
	rp->addInput(render::RGDependency::First);

	if (haveHistory)
	{
		rp->setOutput(maskTargetSetId, render::TfNone, render::TfColor);
		rp->addBuild([=, this](const render::RenderGraph&, render::RenderContext* renderContext) {
			const auto sp = m_reprojectShader->getProgram();
			if (!sp)
				return;

			auto programParams = renderContext->alloc< render::ProgramParameters >();
			programParams->beginParameters(renderContext);
			programParams->setTextureParameter(s_handleDisplacement_PreviousMask, previousMaskTexture);
			programParams->setVectorParameter(s_handleDisplacement_Reproject, Vector4(reprojectU, reprojectV, fade, 0.0f));
			programParams->endParameters(renderContext);

			m_screenRenderer->draw(renderContext, sp.program, programParams, L"Displacement reproject");
		});
	}
	else
	{
		render::Clear clear;
		clear.mask = render::CfColor;
		clear.colors[0] = Color4f(0.0f, 0.0f, 0.0f, 0.0f);
		rp->setOutput(maskTargetSetId, clear, render::TfNone, render::TfColor);
	}

	if (volumeCount > 0)
		rp->addBuild([=, this](const render::RenderGraph&, render::RenderContext* renderContext) {
			const auto sp = m_rasterizeVolumesShader->getProgram();
			if (!sp)
				return;

			auto renderBlock = renderContext->allocNamed< render::IndexedInstancingRenderBlock >(L"Displacement volumes");
			renderBlock->program = sp.program;
			renderBlock->indexBuffer = m_indexBuffer->getBufferView();
			renderBlock->indexType = render::IndexType::UInt16;
			renderBlock->vertexBuffer = m_vertexBuffer->getBufferView();
			renderBlock->vertexLayout = m_vertexLayout;
			renderBlock->primitive = render::PrimitiveType::Triangles;
			renderBlock->offset = 0;
			renderBlock->count = 2;
			renderBlock->instanceCount = volumeCount;

			renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
			renderBlock->programParams->beginParameters(renderContext);
			renderBlock->programParams->setVectorParameter(s_handleDisplacement_MaskExtent, maskExtent);
			renderBlock->programParams->setBufferViewParameter(s_handleDisplacement_Volumes, m_volumeBuffer->getBufferView());
			renderBlock->programParams->endParameters(renderContext);

			renderContext->draw(renderBlock);
		});

	renderGraph.addPass(rp);
}

render::ITexture* DisplacementWorldComponent::getMask() const
{
	return m_haveMask ? m_maskTargetSet[m_current]->getColorTexture(0) : nullptr;
}

void DisplacementWorldComponent::getPermutation(const DisplacementWorldComponent* displacement, const render::Shader* shader, render::Shader::Permutation& inoutPerm)
{
	if (displacement == nullptr)
		return;

	shader->setCombination(s_handleWorld_DisplacementEnable, (bool)(displacement != nullptr), inoutPerm);
}

void DisplacementWorldComponent::setSharedParameters(const DisplacementWorldComponent* displacement, render::ProgramParameters* programParams)
{
	if (displacement == nullptr)
		return;

	render::ITexture* mask = displacement->getMask();
	if (mask == nullptr)
		return;

	programParams->setTextureParameter(s_handleWorld_DisplacementMap, mask);
	programParams->setVectorParameter(s_handleWorld_DisplacementExtent, displacement->getMaskExtent());
}

}
