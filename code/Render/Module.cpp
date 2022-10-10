#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Render/RenderClassFactory.h"
#	include "Render/Image2/AmbientOcclusionData.h"
#	include "Render/Image2/DirectionalBlurData.h"
#	include "Render/Image2/ImagePassData.h"
#	include "Render/Image2/ShadowProjectData.h"
#	include "Render/Image2/SimpleData.h"

namespace traktor::render
{

extern "C" void __module__Traktor_Render()
{
	T_FORCE_LINK_REF(RenderClassFactory);
	T_FORCE_LINK_REF(AmbientOcclusionData);
	T_FORCE_LINK_REF(DirectionalBlurData);
	T_FORCE_LINK_REF(ImagePassData);
	T_FORCE_LINK_REF(ShadowProjectData);
	T_FORCE_LINK_REF(SimpleData);
}

}

#endif
