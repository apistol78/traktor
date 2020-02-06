#include "Render/Editor/Image2/ImgOutput.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

const ImmutableNode::InputPinDesc c_ImgOutput_i[] = { { L"Input", false }, { 0 } };

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ImgOutput", 0, ImgOutput, ImmutableNode)

ImgOutput::ImgOutput()
:	ImmutableNode(c_ImgOutput_i, nullptr)
{
}

void ImgOutput::serialize(ISerializer& s)
{
	Node::serialize(s);
}

	}
}