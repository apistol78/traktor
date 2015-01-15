#ifndef traktor_amalgam_VideoLayer_H
#define traktor_amalgam_VideoLayer_H

#include "Amalgam/Engine/Layer.h"
#include "Core/Math/Aabb2.h"
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
		bool permitTransition,
		amalgam::IEnvironment* environment,
		const resource::Proxy< video::Video >& video,
		const resource::Proxy< render::Shader >& shader,
		const Aabb2& screenBounds,
		bool visible,
		bool autoPlay,
		bool repeat
	);

	virtual ~VideoLayer();

	void destroy();

	void play();

	void stop();

	void rewind();

	void show();

	void hide();

	bool isPlaying() const;

	bool isVisible() const;

	void setScreenBounds(const Aabb2& screenBounds);

	Aabb2 getScreenBounds() const;

	void setRepeat(bool repeat);

	bool getRepeat() const;

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
	Ref< amalgam::IEnvironment > m_environment;
	resource::Proxy< video::Video > m_video;
	resource::Proxy< render::Shader > m_shader;
	Ref< render::ScreenRenderer > m_screenRenderer;
	Aabb2 m_screenBounds;
	bool m_repeat;
	bool m_playing;
	bool m_visible;
};

	}
}

#endif	// traktor_amalgam_VideoLayer_H
