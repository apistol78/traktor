#include "Drawing/CubeMap.h"
#include "Drawing/Image.h"
#include "Drawing/Filters/SphereMapFilter.h"

namespace traktor::drawing
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.drawing.SphereMapFilter", SphereMapFilter, IImageFilter)

void SphereMapFilter::apply(Image* image) const
{
	Ref< CubeMap > cubeMap = CubeMap::createFromImage(image);
	if (!cubeMap)
		return;

	Ref< Image > outputImage = cubeMap->createEquirectangular();
	if (!outputImage)
		return;

	image->swap(outputImage);
}

}
