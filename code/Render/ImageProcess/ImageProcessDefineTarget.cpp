/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Log/Log.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberEnum.h"
#include "Render/IRenderSystem.h"
#include "Render/ImageProcess/ImageProcess.h"
#include "Render/ImageProcess/ImageProcessDefineTarget.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ImageProcessDefineTarget", 4, ImageProcessDefineTarget, ImageProcessDefine)

ImageProcessDefineTarget::ImageProcessDefineTarget()
:	m_width(0)
,	m_height(0)
,	m_screenWidthDenom(0)
,	m_screenHeightDenom(0)
,	m_maxWidth(0)
,	m_maxHeight(0)
,	m_format(TfR8)
,	m_depthStencil(false)
,	m_preferTiled(false)
,	m_multiSample(0)
,	m_clearColor(0.0f, 0.0f, 0.0f, 0.0f)
,	m_persistent(false)
{
}

bool ImageProcessDefineTarget::define(ImageProcess* imageProcess, resource::IResourceManager* resourceManager, IRenderSystem* renderSystem, uint32_t screenWidth, uint32_t screenHeight)
{
	RenderTargetSetCreateDesc rtscd;

	rtscd.count = 1;
	rtscd.width = m_width + (m_screenWidthDenom ? (screenWidth + m_screenWidthDenom - 1) / m_screenWidthDenom : 0);
	rtscd.height = m_height + (m_screenHeightDenom ? (screenHeight + m_screenHeightDenom - 1) / m_screenHeightDenom : 0);
	rtscd.multiSample = m_multiSample;
	rtscd.createDepthStencil = m_depthStencil;
	rtscd.usingPrimaryDepthStencil = false;
	rtscd.ignoreStencil = true;
	rtscd.preferTiled = m_preferTiled;
	rtscd.targets[0].format = m_format;

	if (m_maxWidth > 0)
		rtscd.width = min< int32_t >(rtscd.width, m_maxWidth);
	if (m_maxHeight > 0)
		rtscd.height = min< int32_t >(rtscd.height, m_maxHeight);

	imageProcess->defineTarget(
		m_id,
		getParameterHandle(m_id),
		rtscd,
		m_clearColor,
		m_persistent
	);

	T_DEBUG(L"Post process target \"" << m_id << L"\" (" << rtscd.width << L"*" << rtscd.height << L" " << getTextureFormatName(m_format) << L") created");
	return true;
}

void ImageProcessDefineTarget::serialize(ISerializer& s)
{
	const MemberEnum< TextureFormat >::Key kFormats[] =
	{
		{ L"TfR8", TfR8 },
		{ L"TfR8G8B8A8", TfR8G8B8A8 },
		{ L"TfR16G16B16A16F", TfR16G16B16A16F },
		{ L"TfR32G32B32A32F", TfR32G32B32A32F },
		{ L"TfR16F", TfR16F },
		{ L"TfR32F", TfR32F },
		{ L"TfR11G11B10F", TfR11G11B10F },
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

	s >> MemberEnum< TextureFormat >(L"format", m_format, kFormats);
	s >> Member< bool >(L"depthStencil", m_depthStencil);
	
	if (s.getVersion() >= 1)
		s >> Member< bool >(L"preferTiled", m_preferTiled);
	
	s >> Member< int32_t >(L"multiSample", m_multiSample);

	if (s.getVersion() >= 2)
		s >> Member< Color4f >(L"clearColor", m_clearColor);

	if (s.getVersion() >= 4)
		s >> Member< bool >(L"persistent", m_persistent);
}

	}
}
