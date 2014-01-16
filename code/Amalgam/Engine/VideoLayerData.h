#ifndef traktor_amalgam_VideoLayerData_H
#define traktor_amalgam_VideoLayerData_H

#include "Amalgam/Engine/LayerData.h"
#include "Resource/Id.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AMALGAM_EXPORT)
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

	namespace amalgam
	{

class T_DLLCLASS VideoLayerData : public LayerData
{
	T_RTTI_CLASS;

public:
	virtual Ref< Layer > createInstance(Stage* stage, amalgam::IEnvironment* environment) const;

	virtual void serialize(ISerializer& s);

private:
	friend class StagePipeline;

	resource::Id< video::Video > m_video;
	resource::Id< render::Shader > m_shader;
};

	}
}

#endif	// traktor_amalgam_VideoLayerData_H
