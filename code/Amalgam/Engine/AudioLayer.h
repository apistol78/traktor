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

class T_DLLCLASS AudioLayer : public Layer
{
	T_RTTI_CLASS;

public:
	AudioLayer(
		Stage* stage,
		const std::wstring& name,
		amalgam::IEnvironment* environment,
		const resource::Proxy< sound::Sound >& sound,
		bool autoPlay,
		bool repeat
	);

	virtual ~AudioLayer();

	void destroy();

	void play();

	void stop();

	void fadeOff();

	void setParameter(const std::wstring& parameterName, float value);

	void tweenParameter(const std::wstring& parameterName, float fromValue, float toValue, float duration);

	virtual void prepare();

	virtual void update(amalgam::IUpdateControl& control, const amalgam::IUpdateInfo& info);

	virtual void build(const amalgam::IUpdateInfo& info, uint32_t frame);

	virtual void render(render::EyeType eye, uint32_t frame);

	virtual void preReconfigured();

	virtual void postReconfigured();

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
