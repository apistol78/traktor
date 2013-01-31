#ifndef traktor_parade_AudioLayer_H
#define traktor_parade_AudioLayer_H

#include "Amalgam/IEnvironment.h"
#include "Parade/Layer.h"
#include "Resource/Proxy.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PARADE_EXPORT)
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

	namespace parade
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
		bool autoPlay
	);

	virtual ~AudioLayer();

	void destroy();

	void play();

	void stop();

	void fadeOff();

	virtual void prepare();

	virtual void update(amalgam::IUpdateControl& control, const amalgam::IUpdateInfo& info);

	virtual void build(const amalgam::IUpdateInfo& info, uint32_t frame);

	virtual void render(render::EyeType eye, uint32_t frame);

	virtual void reconfigured();

private:
	Ref< amalgam::IEnvironment > m_environment;
	resource::Proxy< sound::Sound > m_sound;
	Ref< sound::ISoundHandle > m_handle;
	bool m_autoPlay;
};

	}
}

#endif	// traktor_parade_AudioLayer_H
