#ifndef traktor_amalgam_AudioLayer_H
#define traktor_amalgam_AudioLayer_H

#include "Amalgam/Game/IEnvironment.h"
#include "Amalgam/Game/Engine/Layer.h"
#include "Core/Containers/AlignedVector.h"
#include "Resource/Proxy.h"
#include "Sound/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AMALGAM_GAME_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sound
	{

class ISoundHandle;
class Sound;

	}

	namespace amalgam
	{

/*! \brief Stage audio layer.
 * \ingroup Amalgam
 */
class T_DLLCLASS AudioLayer : public Layer
{
	T_RTTI_CLASS;

public:
	AudioLayer(
		Stage* stage,
		const std::wstring& name,
		bool permitTransition,
		IEnvironment* environment,
		const resource::Proxy< sound::Sound >& sound,
		bool autoPlay,
		bool repeat
	);

	virtual void destroy() T_OVERRIDE;

	/*! \brief Play sound. */
	void play();

	/*! \brief Stop sound. */
	void stop();

	/*! \brief Fade off sound. */
	void fadeOff();

	/*! \brief Set sound effect parameter.
	 *
	 * \param parameterName Name of effect parameter.
	 * \param value Effect parameter value.
	 */
	void setParameter(const std::wstring& parameterName, float value);

	/*! \brief Tween effect parameter over a period of time.
	 *
	 * \param parameterName Name of effect parameter.
	 * \param fromValue Effect parameter "from" value.
	 * \param toValue Effect parameter "to" value.
	 * \param duration Duration in seconds for transition.
	 */
	void tweenParameter(const std::wstring& parameterName, float fromValue, float toValue, float duration);

	virtual void transition(Layer* fromLayer) T_OVERRIDE T_FINAL;

	virtual void prepare() T_OVERRIDE T_FINAL;

	virtual void update(const UpdateInfo& info) T_OVERRIDE T_FINAL;

	virtual void build(const UpdateInfo& info, uint32_t frame) T_OVERRIDE T_FINAL;

	virtual void render(render::EyeType eye, uint32_t frame) T_OVERRIDE T_FINAL;

	virtual void flush() T_OVERRIDE T_FINAL;

	virtual void preReconfigured() T_OVERRIDE T_FINAL;

	virtual void postReconfigured() T_OVERRIDE T_FINAL;

	virtual void suspend() T_OVERRIDE T_FINAL;

	virtual void resume() T_OVERRIDE T_FINAL;

private:
	struct Tween
	{
		sound::handle_t parameter;
		float fromValue;
		float toValue;
		float duration;
		float time;
	};

	Ref< IEnvironment > m_environment;
	resource::Proxy< sound::Sound > m_sound;
	Ref< sound::ISoundHandle > m_handle;
	bool m_autoPlay;
	bool m_repeat;
	AlignedVector< Tween > m_tweens;
};

	}
}

#endif	// traktor_amalgam_AudioLayer_H
