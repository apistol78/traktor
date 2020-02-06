#include "Render/Editor/Image2/ImgInput.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

const ImmutableNode::OutputPinDesc c_ImgInput_o[] = { { L"Output" }, { 0 } };

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ImgInput", 0, ImgInput, ImmutableNode)

ImgInput::ImgInput()
:	ImmutableNode(nullptr, c_ImgInput_o)
{
}

void ImgInput::serialize(ISerializer& s)
{
	Node::serialize(s);
}

	}
}