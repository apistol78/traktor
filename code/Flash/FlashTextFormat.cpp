#include "Flash/FlashTextFormat.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.FlashTextFormat", FlashTextFormat, ActionObjectRelay)

FlashTextFormat::FlashTextFormat(float letterSpacing)
:	ActionObjectRelay("TextFormat")
,	m_letterSpacing(letterSpacing)
{
}

	}
}
