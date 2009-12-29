#include "Flash/Action/Avm1/ActionContext.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.ActionContext", ActionContext, Object)

ActionContext::ActionContext(const FlashMovie* movie, ActionObject* global)
:	m_movie(movie)
,	m_global(global)
{
}

	}
}
