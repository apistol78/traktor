#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Render/Editor/Image2/IImgStep.h"
#include "Render/Editor/Image2/ImgPass.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ImgPass", 0, ImgPass, ImmutableNode)

ImgPass::ImgPass()
:	ImmutableNode(nullptr, nullptr)
{
}

const RefArray< IImgStep >& ImgPass::getSteps() const
{
	return m_steps;
}

void ImgPass::serialize(ISerializer& s)
{
	Node::serialize(s);
	s >> MemberRefArray< IImgStep >(L"steps", m_steps);
}

	}
}
