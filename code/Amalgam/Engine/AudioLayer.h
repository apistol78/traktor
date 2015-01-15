#ifndef traktor_amalgam_AudioLayer_H
#define traktor_amalgam_AudioLayer_H

#include "Amalgam/IEnvironment.h"
#include "Amalgam/Engine/Layer.h"
#include "Core/Containers/AlignedVector.h"
#include "Resource/Proxy.h"
#include "Sound/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AMALGAM_EXPORT)
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
		amalgam::IEnvironment* environment,
		const resource::Proxy< sound::Sound >& sound,
		bool autoPlay,
		bool repeat
	);

	virtual ~AudioLayer();

	void destroy();

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

	virtual void transition(Layer* fromLayer);

	virtual void prepare();

	virtual void update(const amalgam::IUpdateInfo& info);

	virtual void build(const amalgam::IUpdateInfo& info, uint32_t frame);

	virtual void render(render::EyeType eye, uint32_t frame);

	virtual void flush();

	virtual void preReconfigured();

	virtual void postReconfigured();

	virtual void suspend();

	virtual void resume();

private:
	struct Tween
	{
		sound::handle_t parameter;
		float fromValue;
		float toValue;
		float duration;
		float time;
	};

	Ref< amalgam::IEnvironment > m_environment;
	resource::Proxy< sound::Sound > m_sound;
	Ref< sound::ISoundHandle > m_handle;
	bool m_autoPlay;
	bool m_repeat;
	AlignedVector< Tween > m_tweens;
};

	}
}

#endif	// traktor_amalgam_AudioLayer_H
