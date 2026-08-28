/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/Entity/ComputeTexture.h"

#include "Core/Misc/SafeDestroy.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Render/IRenderSystem.h"
#include "Render/Shader.h"

namespace traktor::world
{

namespace
{

const render::Handle s_handleComputeTextureOutput(L"ComputeTexture_Output");
const render::Handle s_handleComputeTextureSize(L"ComputeTexture_Size");
const render::Handle s_handleComputeTextureTime(L"ComputeTexture_Time");

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.ComputeTexture", ComputeTexture, render::ITexture)

bool ComputeTexture::create(
	render::IRenderSystem* renderSystem,
	const resource::Proxy< render::Shader >& shader,
	int32_t width,
	int32_t height,
	render::TextureFormat format,
	bool continuous)
{
	render::SimpleTextureCreateDesc desc;
	desc.width = width;
	desc.height = height;
	desc.mipCount = 1;
	desc.format = format;
	desc.sRGB = false;
	desc.immutable = false;
	desc.shaderStorage = true;

	m_texture = renderSystem->createSimpleTexture(desc, T_FILE_LINE_W);
	if (!m_texture)
		return false;

	m_shader = shader;
	m_width = width;
	m_height = height;
	m_continuous = continuous;
	m_generated = false;

	m_timer.reset();

	return true;
}

void ComputeTexture::destroy()
{
	safeDestroy(m_texture);
}

render::ITexture::Size ComputeTexture::getSize() const
{
	return m_texture ? m_texture->getSize() : Size{ 0, 0, 0, 0 };
}

int32_t ComputeTexture::getBindlessIndex() const
{
	return m_texture ? m_texture->getBindlessIndex() : -1;
}

bool ComputeTexture::lock(int32_t side, int32_t level, Lock& lock)
{
	return false;
}

void ComputeTexture::unlock(int32_t side, int32_t level)
{
}

render::ITexture* ComputeTexture::resolve()
{
	return m_texture ? m_texture->resolve() : nullptr;
}

void ComputeTexture::setup(render::RenderGraph& renderGraph)
{
	// If not continuous and already generated, skip dispatch.
	if (!m_continuous && m_generated)
		return;

	const render::RGDependency dependency = renderGraph.addDependency();

	Ref< render::RenderPass > rp = new render::RenderPass(L"Compute Texture");
	rp->addInput(render::RGDependency::First);
	rp->setOutput(dependency);

	const float elapsedTime = (float)m_timer.getElapsedTime();

	rp->addBuild(
		[=](const render::RenderGraph& renderGraph, render::RenderContext* renderContext) {
		const auto sp = m_shader->getProgram();
		if (!sp.program)
			return;

		auto renderBlock = renderContext->allocNamed< render::ComputeRenderBlock >(L"Compute Texture");

		renderBlock->program = sp.program;
		renderBlock->workSize[0] = m_width;
		renderBlock->workSize[1] = m_height;
		renderBlock->workSize[2] = 1;

		renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
		renderBlock->programParams->beginParameters(renderContext);
		renderBlock->programParams->setImageViewParameter(s_handleComputeTextureOutput, m_texture, 0);
		renderBlock->programParams->setVectorParameter(s_handleComputeTextureSize, Vector4(float(m_width), float(m_height), 1.0f / m_width, 1.0f / m_height));
		renderBlock->programParams->setFloatParameter(s_handleComputeTextureTime, elapsedTime);
		renderBlock->programParams->endParameters(renderContext);

		renderContext->compute(renderBlock);
		renderContext->compute< render::BarrierRenderBlock >(render::Stage::Compute, render::Stage::Fragment, m_texture, 0);
	});

	renderGraph.addPass(rp);
	m_generated = true;
}

}
