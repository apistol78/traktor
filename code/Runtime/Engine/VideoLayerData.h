#pragma once

#include "Runtime/Engine/LayerData.h"
#include "Core/Math/Aabb2.h"
#include "Resource/Id.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RUNTIME_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class Shader;

	}

	namespace video
	{

class Video;

	}

	namespace runtime
	{

/*! Video playback layer.
 * \ingroup Runtime
 */
class T_DLLCLASS VideoLayerData : public LayerData
{
	T_RTTI_CLASS;

public:
	virtual Ref< Layer > createInstance(Stage* stage, IEnvironment* environment) const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	friend class StagePipeline;

	resource::Id< video::Video > m_video;
	resource::Id< render::Shader > m_shader;
	Aabb2 m_screenBounds = { Vector2(0.0f, 0.0f), Vector2(1.0f, 1.0f) };
	bool m_visible = true;
	bool m_autoPlay = true;
	bool m_repeat = true;
};

	}
}
