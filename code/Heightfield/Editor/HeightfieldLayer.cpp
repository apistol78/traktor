#include "Heightfield/Editor/HeightfieldLayer.h"

namespace traktor
{
	namespace hf
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.hf.HeightfieldLayer", HeightfieldLayer, Object)

HeightfieldLayer::HeightfieldLayer(drawing::Image* image)
:	m_image(image)
{
}

	}
}
