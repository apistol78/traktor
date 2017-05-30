/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_ActionContext_H
#define traktor_flash_ActionContext_H

#include "Core/Containers/AlignedVector.h"
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

class CharacterInstance;
class Movie;
class SpriteInstance;
class ICharacterFactory;
class IMovieLoader;

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
		IdParent = 10,
		IdApply = 11,
		IdCall = 12,
		IdToString = 13,
		IdTarget = 14,
		IdTargetProperty = 15,
		IdFunction = 16,
		IdOnLoad = 17,
		IdOnEnterFrame = 18,
		IdOnKeyDown = 19,
		IdOnKeyUp = 20,
		IdOnMouseDown = 21,
		IdOnPress = 22,
		IdOnMouseUp = 23,
		IdOnRelease = 24,
		IdOnMouseMove = 25,
		IdOnRollOver = 26,
		IdOnRollOut = 27,
		IdOnFrame = 28,
		IdOnSetFocus = 29,
		IdOnKillFocus = 30,
		IdOnMouseWheel = 31,
		IdOnChanged = 32,
		IdOnScroller = 33,
		IdX = 34,
		IdY = 35,
		IdWidth = 36,
		IdHeight = 37
	};

	ActionContext(const Movie* movie, const ICharacterFactory* characterFactory, const IMovieLoader* movieLoader);

	void setGlobal(ActionObject* global);

	void setMovieClip(SpriteInstance* movieClip);

	void setFocus(CharacterInstance* focus);

	void setPressed(CharacterInstance* pressed);

	void setRolledOver(CharacterInstance* rolledOver);

	void addFrameListener(ActionObject* frameListener);

	void removeFrameListener(ActionObject* frameListener);

	void notifyFrameListeners(float time);

	ActionObject* lookupClass(const std::string& className);

	uint32_t getString(const std::string& str);

	std::string getString(uint32_t id);

	const Movie* getMovie() const { return m_movie; }

	const ICharacterFactory* getCharacterFactory() const { return m_characterFactory; }

	const IMovieLoader* getMovieLoader() const { return m_movieLoader; }

	ActionObject* getGlobal() const { return m_global; }

	SpriteInstance* getMovieClip() const { return m_movieClip; }

	CharacterInstance* getFocus() const { return m_focus; }

	CharacterInstance* getPressed() const { return m_pressed; }

	CharacterInstance* getRolledOver() const { return m_rolledOver; }

	ActionValuePool& getPool() { return m_pool; }

protected:
	virtual void trace(visitor_t visitor) const T_OVERRIDE;

	virtual void dereference() T_OVERRIDE;

private:
	struct FrameListener
	{
		Ref< ActionObject > listenerTarget;
		Ref< ActionFunction > listenerFunction;
	};

	const Movie* m_movie;
	Ref< const ICharacterFactory > m_characterFactory;
	Ref< const IMovieLoader > m_movieLoader;
	Ref< ActionObject > m_global;
	Ref< SpriteInstance > m_movieClip;
	Ref< CharacterInstance > m_focus;
	Ref< CharacterInstance > m_pressed;
	Ref< CharacterInstance > m_rolledOver;
	ActionStrings m_strings;
	ActionValuePool m_pool;
	AlignedVector< FrameListener > m_frameListeners;
};

	}
}

#endif	// traktor_flash_ActionContext_H
