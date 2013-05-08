#include "Core/Log/Log.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberEnum.h"
#include "Render/IRenderSystem.h"
#include "World/PostProcess/PostProcess.h"
#include "World/PostProcess/PostProcessDefineTarget.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.PostProcessDefineTarget", 3, PostProcessDefineTarget, PostProcessDefine)

PostProcessDefineTarget::PostProcessDefineTarget()
:	m_width(0)
,	m_height(0)
,	m_screenWidthDenom(0)
,	m_screenHeightDenom(0)
,	m_maxWidth(0)
,	m_maxHeight(0)
,	m_format(render::TfInvalid)
,	m_depthStencil(false)
,	m_preferTiled(false)
,	m_multiSample(0)
,	m_clearColor(0.0f, 0.0f, 0.0f, 0.0f)
{
}

bool PostProcessDefineTarget::define(PostProcess* postProcess, render::IRenderSystem* renderSystem, uint32_t screenWidth, uint32_t screenHeight)
{
	render::RenderTargetSetCreateDesc desc;

	desc.count = 1;
	desc.width = m_width + (m_screenWidthDenom ? (screenWidth + m_screenWidthDenom - 1) / m_screenWidthDenom : 0);
	desc.height = m_height + (m_screenHeightDenom ? (screenHeight + m_screenHeightDenom - 1) / m_screenHeightDenom : 0);
	desc.multiSample = m_multiSample;
	desc.createDepthStencil = m_depthStencil;
	desc.usingPrimaryDepthStencil = false;
	desc.preferTiled = m_preferTiled;
	desc.targets[0].format = m_format;

	if (m_maxWidth > 0)
		desc.width = min< int32_t >(desc.width, m_maxWidth);
	if (m_maxHeight > 0)
		desc.height = min< int32_t >(desc.height, m_maxHeight);

	Ref< render::RenderTargetSet > renderTargetSet = renderSystem->createRenderTargetSet(desc);
	if (!renderTargetSet)
	{
		log::error << L"Unable to create render target (" << desc.width << L"*" << desc.height << L", msaa " << desc.multiSample << L")" << Endl;
		return false;
	}

	postProcess->defineTarget(
		render::getParameterHandle(m_id),
		renderTargetSet,
		m_clearColor
	);

	T_DEBUG(L"Post process target \"" << m_id << L"\" " << desc.width << L"*" << desc.height << L" created");
	return true;
}

void PostProcessDefineTarget::serialize(ISerializer& s)
{
	const MemberEnum< render::TextureFormat >::Key kFormats[] =
	{
		{ L"TfR8", render::TfR8 },
		{ L"TfR8G8B8A8", render::TfR8G8B8A8 },
		{ L"TfR16G16B16A16F", render::TfR16G16B16A16F },
		{ L"TfR32G32B32A32F", render::TfR32G32B32A32F },
		{ L"TfR16F", render::TfR16F },
		{ L"TfR32F", render::TfR32F },
		{ L"TfR11G11B10F", render::TfR11G11B10F },
		{ 0, 0 }
	};

	s >> Member< std::wstring >(L"id", m_id);
	s >> Member< uint32_t >(L"width", m_width);
	s >> Member< uint32_t >(L"height", m_height);

	if (s.getVersion() >= 3)
	{
		s >> Member< uint32_t >(L"screenWidthDenom", m_screenWidthDenom);
		s >> Member< uint32_t >(L"screenHeightDenom", m_screenHeightDenom);
		s >> Member< uint32_t >(L"maxWidth", m_maxWidth);
		s >> Member< uint32_t >(L"maxHeight", m_maxHeight);
	}
	else
	{
		uint32_t screenDenom = 0;
		s >> Member< uint32_t >(L"screenDenom", screenDenom);
		m_screenWidthDenom = screenDenom;
		m_screenHeightDenom = screenDenom;
	}

	s >> MemberEnum< render::TextureFormat >(L"format", m_format, kFormats);
	s >> Member< bool >(L"depthStencil", m_depthStencil);
	
	if (s.getVersion() >= 1)
		s >> Member< bool >(L"preferTiled", m_preferTiled);
	
	s >> Member< int32_t >(L"multiSample", m_multiSample);

	if (s.getVersion() >= 2)
		s >> Member< Color4f >(L"clearColor", m_clearColor);
}

	}
}
