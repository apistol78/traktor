/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_spark_SoundComponent_H
#define traktor_spark_SoundComponent_H

#include "Core/Ref.h"
#include "Core/Containers/SmallMap.h"
#include "Resource/Proxy.h"
#include "Spark/IComponent.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sound
	{

class ISoundHandle;
class ISoundPlayer;
class Sound;

	}

	namespace spark
	{

/*! \brief Sound component instance.
 * \ingroup Spark
 */
class T_DLLCLASS SoundComponent : public IComponent
{
	T_RTTI_CLASS;

public:
	SoundComponent(sound::ISoundPlayer* soundPlayer, const SmallMap< std::wstring, resource::Proxy< sound::Sound > >& sounds);

	Ref< sound::ISoundHandle > play(const std::wstring& id);

	virtual void update() T_OVERRIDE T_FINAL;

	virtual void eventKey(wchar_t unicode) T_OVERRIDE T_FINAL;

	virtual void eventKeyDown(int32_t keyCode) T_OVERRIDE T_FINAL;

	virtual void eventKeyUp(int32_t keyCode) T_OVERRIDE T_FINAL;

	virtual void eventMouseDown(const Vector2& position, int32_t button) T_OVERRIDE T_FINAL;

	virtual void eventMouseUp(const Vector2& position, int32_t button) T_OVERRIDE T_FINAL;

	virtual void eventMousePress(const Vector2& position, int32_t button) T_OVERRIDE T_FINAL;

	virtual void eventMouseRelease(const Vector2& position, int32_t button) T_OVERRIDE T_FINAL;

	virtual void eventMouseMove(const Vector2& position, int32_t button) T_OVERRIDE T_FINAL;

	virtual void eventMouseEnter(const Vector2& position, int32_t button) T_OVERRIDE T_FINAL;

	virtual void eventMouseLeave(const Vector2& position, int32_t button) T_OVERRIDE T_FINAL;

	virtual void eventMouseWheel(const Vector2& position, int32_t delta) T_OVERRIDE T_FINAL;

	virtual void eventViewResize(int32_t width, int32_t height) T_OVERRIDE T_FINAL;

private:
	Ref< sound::ISoundPlayer > m_soundPlayer;
	SmallMap< std::wstring, resource::Proxy< sound::Sound > > m_sounds;
};

	}
}

#endif	// traktor_spark_SoundComponent_H
