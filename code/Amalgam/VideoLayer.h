#ifndef traktor_amalgam_VideoLayer_H
#define traktor_amalgam_VideoLayer_H

#include "Amalgam/Layer.h"
#include "Resource/Proxy.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AMALGAM_EXPORT)
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

	namespace amalgam
	{

class T_DLLCLASS VideoLayer : public Layer
{
	T_RTTI_CLASS;

public:
	VideoLayer(
		Stage* stage,
		const std::wstring& name,
		amalgam::IEnvironment* environment,
		const resource::Proxy< video::Video >& video,
		const resource::Proxy< render::Shader >& shader
	);

	virtual ~VideoLayer();

	void destroy();

	bool playing() const;

	virtual void prepare();

	virtual void update(amalgam::IUpdateControl& control, const amalgam::IUpdateInfo& info);

	virtual void build(const amalgam::IUpdateInfo& info, uint32_t frame);

	virtual void render(render::EyeType eye, uint32_t frame);

	virtual void preReconfigured();

	virtual void postReconfigured();

private:
	Ref< amalgam::IEnvironment > m_environment;
	resource::Proxy< video::Video > m_video;
	resource::Proxy< render::Shader > m_shader;
	Ref< render::ScreenRenderer > m_screenRenderer;
};

	}
}

#endif	// traktor_amalgam_VideoLayer_H
