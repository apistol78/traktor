#ifndef traktor_flash_ActionContext_H
#define traktor_flash_ActionContext_H

#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Flash/Action/ActionValue.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace flash
	{

class FlashMovie;
class IActionVM;

/*! \brief ActionScript execution context.
 * \ingroup Flash
 */
class T_DLLCLASS ActionContext : public Object
{
	T_RTTI_CLASS;

public:
	ActionContext(const FlashMovie* movie, ActionObject* global);

	void addFrameListener(ActionObject* frameListener);

	void removeFrameListener(ActionObject* frameListener);

	void notifyFrameListeners(const IActionVM* vm, avm_number_t time);

	inline Ref< const FlashMovie > getMovie() const { return m_movie; }

	inline Ref< ActionObject > getGlobal() const { return m_global; }

private:
	struct FrameListener
	{
		Ref< ActionObject > listenerTarget;
		Ref< ActionFunction > listenerFunction;
	};

	Ref< const FlashMovie > m_movie;
	Ref< ActionObject > m_global;
	std::vector< FrameListener > m_frameListeners;
};

	}
}

#endif	// traktor_flash_ActionContext_H
