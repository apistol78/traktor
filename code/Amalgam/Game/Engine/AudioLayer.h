#pragma once

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

	virtual void destroy() override;

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

	virtual void transition(Layer* fromLayer) override final;

	virtual void prepare(const UpdateInfo& info) override final;

	virtual void update(const UpdateInfo& info) override final;

	virtual void build(const UpdateInfo& info, uint32_t frame) override final;

	virtual void render(render::EyeType eye, uint32_t frame) override final;

	virtual void flush() override final;

	virtual void preReconfigured() override final;

	virtual void postReconfigured() override final;

	virtual void suspend() override final;

	virtual void resume() override final;

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

