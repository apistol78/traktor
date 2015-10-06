#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Input/InputClassFactory.h"
#	include "Input/Binding/CombinedInputSourceData.h"
#	include "Input/Binding/ConstantInputSourceData.h"
#	include "Input/Binding/GenericInputSourceData.h"
#	include "Input/Binding/IfInclusiveExclusive.h"
#	include "Input/Binding/InBoolean.h"
#	include "Input/Binding/InClamp.h"
#	include "Input/Binding/InCombine.h"
#	include "Input/Binding/InConst.h"
#	include "Input/Binding/InDelay.h"
#	include "Input/Binding/InGesturePinch.h"
#	include "Input/Binding/InGestureSwipe.h"
#	include "Input/Binding/InGestureTap.h"
#	include "Input/Binding/InHysteresis.h"
#	include "Input/Binding/InPulse.h"
#	include "Input/Binding/InputMappingSourceData.h"
#	include "Input/Binding/InputMappingStateData.h"
#	include "Input/Binding/InputStateData.h"
#	include "Input/Binding/InReadValue.h"
#	include "Input/Binding/InRemapAxis.h"
#	include "Input/Binding/InThreshold.h"
#	include "Input/Binding/InTrigger.h"
#	include "Input/Binding/KeyboardInputSourceData.h"

namespace traktor
{
	namespace input
	{

extern "C" void __module__Traktor_Input()
{
	T_FORCE_LINK_REF(InputClassFactory);
	T_FORCE_LINK_REF(CombinedInputSourceData);
	T_FORCE_LINK_REF(ConstantInputSourceData);
	T_FORCE_LINK_REF(GenericInputSourceData);
	T_FORCE_LINK_REF(IfInclusiveExclusive);
	T_FORCE_LINK_REF(InBoolean);
	T_FORCE_LINK_REF(InClamp);
	T_FORCE_LINK_REF(InCombine);
	T_FORCE_LINK_REF(InConst);
	T_FORCE_LINK_REF(InDelay);
	T_FORCE_LINK_REF(InGesturePinch);
	T_FORCE_LINK_REF(InGestureSwipe);
	T_FORCE_LINK_REF(InGestureTap);
	T_FORCE_LINK_REF(InHysteresis);
	T_FORCE_LINK_REF(InPulse);
	T_FORCE_LINK_REF(InputMappingSourceData);
	T_FORCE_LINK_REF(InputMappingStateData);
	T_FORCE_LINK_REF(InputStateData);
	T_FORCE_LINK_REF(InReadValue);
	T_FORCE_LINK_REF(InRemapAxis);
	T_FORCE_LINK_REF(InThreshold);
	T_FORCE_LINK_REF(InTrigger);
	T_FORCE_LINK_REF(KeyboardInputSourceData);
}

	}
}

#endif
