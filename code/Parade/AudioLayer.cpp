#include "Parade/AudioLayer.h"

namespace traktor
{
	namespace parade
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.parade.AudioLayer", AudioLayer, Layer)

AudioLayer::AudioLayer(
	const std::wstring& name,
	amalgam::IEnvironment* environment,
	const resource::Proxy< script::IScriptContext >& scriptContext
)
:	Layer(name, scriptContext)
,	m_environment(environment)
{
}

void AudioLayer::update(Stage* stage, amalgam::IUpdateControl& control, const amalgam::IUpdateInfo& info)
{
	// Issue script update method.
	invokeScriptUpdate(stage, control, info);
}

void AudioLayer::build(Stage* stage, const amalgam::IUpdateInfo& info, uint32_t frame)
{
}

void AudioLayer::render(Stage* stage, render::EyeType eye, uint32_t frame)
{
}

void AudioLayer::leave(Stage* stage)
{
}

void AudioLayer::reconfigured(Stage* stage)
{
}

	}
}
