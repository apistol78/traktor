/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Render/ImageProcess/ImageProcessDefine.h"
#include "Render/ImageProcess/ImageProcessSettings.h"
#include "Render/ImageProcess/ImageProcessStep.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.render.ImageProcessSettings", 1, ImageProcessSettings, ISerializable)

ImageProcessSettings::ImageProcessSettings()
:	m_requireHighRange(false)
{
}

bool ImageProcessSettings::requireHighRange() const
{
	return m_requireHighRange;
}

void ImageProcessSettings::setDefinitions(const RefArray< ImageProcessDefine >& definitions)
{
	m_definitions = definitions;
}

const RefArray< ImageProcessDefine >& ImageProcessSettings::getDefinitions() const
{
	return m_definitions;
}

void ImageProcessSettings::setSteps(const RefArray< ImageProcessStep >& steps)
{
	m_steps = steps;
}

const RefArray< ImageProcessStep >& ImageProcessSettings::getSteps() const
{
	return m_steps;
}

void ImageProcessSettings::serialize(ISerializer& s)
{
	if (s.getVersion< ImageProcessSettings >() >= 1)
		s >> Member< bool >(L"requireHighRange", m_requireHighRange);

	s >> MemberRefArray< ImageProcessDefine >(L"definitions", m_definitions);
	s >> MemberRefArray< ImageProcessStep >(L"steps", m_steps);
}

	}
}
