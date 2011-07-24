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
class FlashSpriteInstance;
class IActionVM;

/*! \brief ActionScript execution context.
 * \ingroup Flash
 */
class T_DLLCLASS ActionContext : public Object
{
	T_RTTI_CLASS;

public:
	ActionContext(const IActionVM* vm, const FlashMovie* movie, ActionObject* global);

	void addFrameListener(ActionObject* frameListener);

	void removeFrameListener(ActionObject* frameListener);

	void notifyFrameListeners(avm_number_t time);

	ActionObject* lookupClass(const std::string& className);

	void pushMovieClip(FlashSpriteInstance* spriteInstance);

	void popMovieClip();

	FlashSpriteInstance* getMovieClip();

	const IActionVM* getVM() const { return m_vm; }

	const FlashMovie* getMovie() const { return m_movie; }

	const Ref< ActionObject >& getGlobal() const { return m_global; }

	ActionValuePool& getPool() { return m_pool; }

private:
	struct FrameListener
	{
		Ref< ActionObject > listenerTarget;
		Ref< ActionFunction > listenerFunction;
	};

	const IActionVM* m_vm;
	const FlashMovie* m_movie;
	RefArray< FlashSpriteInstance > m_movieClipStack;
	Ref< ActionObject > m_global;
	ActionValuePool m_pool;
	std::vector< FrameListener > m_frameListeners;
};

	}
}

#endif	// traktor_flash_ActionContext_H
