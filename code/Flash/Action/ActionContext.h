#ifndef traktor_flash_ActionContext_H
#define traktor_flash_ActionContext_H

#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Flash/Action/ActionValue.h"
#include "Flash/Action/ActionValuePool.h"

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

	const Ref< const FlashMovie >& getMovie() const { return m_movie; }

	const Ref< ActionObject >& getGlobal() const { return m_global; }

	ActionValuePool& getPool() { return m_pool; }

private:
	struct FrameListener
	{
		Ref< ActionObject > listenerTarget;
		Ref< ActionFunction > listenerFunction;
	};

	Ref< const FlashMovie > m_movie;
	Ref< ActionObject > m_global;
	ActionValuePool m_pool;
	std::vector< FrameListener > m_frameListeners;
};

	}
}

#endif	// traktor_flash_ActionContext_H
