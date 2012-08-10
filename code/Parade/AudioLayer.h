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

class Sound;
class SoundChannel;

	}

	namespace parade
	{

class T_DLLCLASS AudioLayer : public Layer
{
	T_RTTI_CLASS;

public:
	AudioLayer(
		const std::wstring& name,
		amalgam::IEnvironment* environment,
		const resource::Proxy< script::IScriptContext >& scriptContext,
		const resource::Proxy< sound::Sound >& sound
	);

	virtual void update(Stage* stage, amalgam::IUpdateControl& control, const amalgam::IUpdateInfo& info);

	virtual void build(Stage* stage, const amalgam::IUpdateInfo& info, uint32_t frame);

	virtual void render(Stage* stage, render::EyeType eye, uint32_t frame);

	virtual void leave(Stage* stage);

	virtual void reconfigured(Stage* stage);

private:
	Ref< amalgam::IEnvironment > m_environment;
	resource::Proxy< sound::Sound > m_sound;
	Ref< sound::SoundChannel > m_soundChannel;
};

	}
}

#endif	// traktor_parade_AudioLayer_H
