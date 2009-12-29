#ifndef traktor_flash_ActionContext_H
#define traktor_flash_ActionContext_H

#include "Core/Object.h"
#include "Flash/Action/Avm1/ActionValue.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace flash
	{

class FlashMovie;

/*! \brief ActionScript execution context.
 * \ingroup Flash
 */
class T_DLLCLASS ActionContext : public Object
{
	T_RTTI_CLASS;

public:
	ActionContext(const FlashMovie* movie, ActionObject* global);

	inline Ref< const FlashMovie > getMovie() const { return m_movie; }

	inline Ref< ActionObject > getGlobal() const { return m_global; }

private:
	Ref< const FlashMovie > m_movie;
	Ref< ActionObject > m_global;
};

	}
}

#endif	// traktor_flash_ActionContext_H
