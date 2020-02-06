#include "Render/Editor/Image2/ImageGraphAsset.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.render.ImageGraphAsset", 0, ImageGraphAsset, ISerializable)

ImageGraphAsset::ImageGraphAsset()
:	Graph()
{
}

ImageGraphAsset::ImageGraphAsset(const RefArray< Node >& nodes, const RefArray< Edge >& edges)
:	Graph(nodes, edges)
{
}

	}
}
