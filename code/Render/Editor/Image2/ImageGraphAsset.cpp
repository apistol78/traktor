#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Render/Editor/Image2/IgaPass.h"
#include "Render/Editor/Image2/IgaTarget.h"
#include "Render/Editor/Image2/ImageGraphAsset.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.render.ImageGraphAsset", 0, ImageGraphAsset, ISerializable)

void ImageGraphAsset::addTarget(IgaTarget* target)
{
	m_targets.push_back(target);
}

const RefArray< IgaTarget >& ImageGraphAsset::getTargets() const
{
	return m_targets;
}

void ImageGraphAsset::addPass(IgaPass* pass)
{
	m_passes.push_back(pass);
}

const RefArray< IgaPass >& ImageGraphAsset::getPasses() const
{
	return m_passes;
}

void ImageGraphAsset::serialize(ISerializer& s)
{
	s >> MemberRefArray< IgaTarget >(L"targets", m_targets);
	s >> MemberRefArray< IgaPass >(L"passes", m_passes);
}

	}
}