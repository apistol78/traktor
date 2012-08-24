#ifndef traktor_parade_VideoLayer_H
#define traktor_parade_VideoLayer_H

#include "Resource/Proxy.h"
#include "Parade/Layer.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PARADE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace amalgam
	{

class IEnvironment;

	}

	namespace render
	{

class ScreenRenderer;
class Shader;

	}

	namespace video
	{

class Video;

	}

	namespace parade
	{

class T_DLLCLASS VideoLayer : public Layer
{
	T_RTTI_CLASS;

public:
	VideoLayer(
		const std::wstring& name,
		amalgam::IEnvironment* environment,
		const resource::Proxy< script::IScriptContext >& scriptContext,
		const resource::Proxy< video::Video >& video,
		const resource::Proxy< render::Shader >& shader
	);

	virtual void prepare(Stage* stage);

	virtual void update(Stage* stage, amalgam::IUpdateControl& control, const amalgam::IUpdateInfo& info);

	virtual void build(Stage* stage, const amalgam::IUpdateInfo& info, uint32_t frame);

	virtual void render(Stage* stage, render::EyeType eye, uint32_t frame);

	virtual void leave(Stage* stage);

	virtual void reconfigured(Stage* stage);

private:
	Ref< amalgam::IEnvironment > m_environment;
	resource::Proxy< video::Video > m_video;
	resource::Proxy< render::Shader > m_shader;
	Ref< render::ScreenRenderer > m_screenRenderer;
};

	}
}

#endif	// traktor_parade_VideoLayer_H
