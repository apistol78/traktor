#ifndef traktor_flash_ActionContext_H
#define traktor_flash_ActionContext_H

#include "Flash/Collectable.h"
#include "Flash/Action/ActionStrings.h"
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
	// Predefined strings.
	enum StringIds
	{
		IdEmpty = 0,
		Id__proto__ = 1,
		IdPrototype = 2,
		Id__ctor__ = 3,
		IdConstructor = 4,
		IdThis = 5,
		IdSuper = 6,
		IdGlobal = 7,
		IdArguments = 8,
		IdRoot = 9,
		IdParent = 10
	};

	ActionContext(const IActionVM* vm, const FlashMovie* movie);

	void setGlobal(ActionObject* global);

	void setMovieClip(FlashSpriteInstance* movieClip);

	void addFrameListener(ActionObject* frameListener);

	void removeFrameListener(ActionObject* frameListener);

	void notifyFrameListeners(avm_number_t time);

	ActionObject* lookupClass(const std::string& className);

	uint32_t getString(const std::string& str);

	std::string getString(uint32_t id);

	const IActionVM* getVM() const { return m_vm; }

	const FlashMovie* getMovie() const { return m_movie; }

	ActionObject* getGlobal() const { return m_global; }

	FlashSpriteInstance* getMovieClip() const { return m_movieClip; }

	ActionValuePool& getPool() { return m_pool; }

protected:
	virtual void trace(const IVisitor& visitor) const;

	virtual void dereference();

private:
	struct FrameListener
	{
		Ref< ActionObject > listenerTarget;
		Ref< ActionFunction > listenerFunction;
	};

	const IActionVM* m_vm;
	const FlashMovie* m_movie;
	Ref< ActionObject > m_global;
	Ref< FlashSpriteInstance > m_movieClip;
	ActionStrings m_strings;
	ActionValuePool m_pool;
	std::vector< FrameListener > m_frameListeners;
	uint32_t m_idOnFrame;
	uint32_t m_idPrototype;
};

	}
}

#endif	// traktor_flash_ActionContext_H
