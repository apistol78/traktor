#ifndef traktor_flash_ActionContext_H
#define traktor_flash_ActionContext_H

#include "Flash/Collectable.h"
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
class T_DLLCLASS ActionContext : public Collectable
{
	T_RTTI_CLASS;

public:
	ActionContext(const IActionVM* vm, const FlashMovie* movie);

	void setGlobal(ActionObject* global);

	void setMovieClip(FlashSpriteInstance* movieClip);

	ActionObject* lookupClass(const std::string& className);

	const IActionVM* getVM() const { return m_vm; }

	const FlashMovie* getMovie() const { return m_movie; }

	ActionObject* getGlobal() const { return m_global; }

	FlashSpriteInstance* getMovieClip() const { return m_movieClip; }

	ActionValuePool& getPool() { return m_pool; }

protected:
	virtual void trace(const IVisitor& visitor) const;

	virtual void dereference();

private:
	const IActionVM* m_vm;
	const FlashMovie* m_movie;
	Ref< ActionObject > m_global;
	Ref< FlashSpriteInstance > m_movieClip;
	ActionValuePool m_pool;
};

	}
}

#endif	// traktor_flash_ActionContext_H
