#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Render/Image/ImageProcessDefine.h"
#include "Render/Image/ImageProcessData.h"
#include "Render/Image/ImageProcessStep.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.render.ImageProcessData", 1, ImageProcessData, ISerializable)

ImageProcessData::ImageProcessData()
:	m_requireHighRange(false)
{
}

bool ImageProcessData::requireHighRange() const
{
	return m_requireHighRange;
}

void ImageProcessData::setDefinitions(const RefArray< ImageProcessDefine >& definitions)
{
	m_definitions = definitions;
}

const RefArray< ImageProcessDefine >& ImageProcessData::getDefinitions() const
{
	return m_definitions;
}

void ImageProcessData::setSteps(const RefArray< ImageProcessStep >& steps)
{
	m_steps = steps;
}

const RefArray< ImageProcessStep >& ImageProcessData::getSteps() const
{
	return m_steps;
}

void ImageProcessData::serialize(ISerializer& s)
{
	if (s.getVersion() >= 1)
		s >> Member< bool >(L"requireHighRange", m_requireHighRange);

	s >> MemberRefArray< ImageProcessDefine >(L"definitions", m_definitions);
	s >> MemberRefArray< ImageProcessStep >(L"steps", m_steps);
}

	}
}
