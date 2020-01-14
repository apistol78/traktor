#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Render/RenderClassFactory.h"
#	include "Render/Image/Defines/ImageProcessDefineTarget.h"
#	include "Render/Image/Defines/ImageProcessDefineTexture.h"
#	include "Render/Image/Steps/ImageProcessStepBlur.h"
#	include "Render/Image/Steps/ImageProcessStepBokeh.h"
#	include "Render/Image/Steps/ImageProcessStepChain.h"
#	include "Render/Image/Steps/ImageProcessStepDiscardTarget.h"
#	include "Render/Image/Steps/ImageProcessStepGodRay.h"
#	include "Render/Image/Steps/ImageProcessStepGrain.h"
#	include "Render/Image/Steps/ImageProcessStepLensDirt.h"
#	include "Render/Image/Steps/ImageProcessStepLuminance.h"
#	include "Render/Image/Steps/ImageProcessStepRepeat.h"
#	include "Render/Image/Steps/ImageProcessStepSetTarget.h"
#	include "Render/Image/Steps/ImageProcessStepSimple.h"
#	include "Render/Image/Steps/ImageProcessStepSmProj.h"
#	include "Render/Image/Steps/ImageProcessStepSsao.h"
#	include "Render/Image/Steps/ImageProcessStepSwapTargets.h"
#	include "Render/Image/Steps/ImageProcessStepTemporal.h"

namespace traktor
{
	namespace render
	{

extern "C" void __module__Traktor_Render()
{
	T_FORCE_LINK_REF(RenderClassFactory);
	T_FORCE_LINK_REF(ImageProcessDefineTarget);
	T_FORCE_LINK_REF(ImageProcessDefineTexture);
	T_FORCE_LINK_REF(ImageProcessStepBlur);
	T_FORCE_LINK_REF(ImageProcessStepBokeh);
	T_FORCE_LINK_REF(ImageProcessStepChain);
	T_FORCE_LINK_REF(ImageProcessStepDiscardTarget);
	T_FORCE_LINK_REF(ImageProcessStepGodRay);
	T_FORCE_LINK_REF(ImageProcessStepGrain);
	T_FORCE_LINK_REF(ImageProcessStepLensDirt);
	T_FORCE_LINK_REF(ImageProcessStepLuminance);
	T_FORCE_LINK_REF(ImageProcessStepRepeat);
	T_FORCE_LINK_REF(ImageProcessStepSetTarget);
	T_FORCE_LINK_REF(ImageProcessStepSimple);
	T_FORCE_LINK_REF(ImageProcessStepSmProj);
	T_FORCE_LINK_REF(ImageProcessStepSsao);
	T_FORCE_LINK_REF(ImageProcessStepSwapTargets);
	T_FORCE_LINK_REF(ImageProcessStepTemporal);
}

	}
}

#endif
