#ifndef traktor_parade_VideoLayerData_H
#define traktor_parade_VideoLayerData_H

#include "Resource/Id.h"
#include "Parade/LayerData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PARADE_EXPORT)
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

	namespace parade
	{

class T_DLLCLASS VideoLayerData : public LayerData
{
	T_RTTI_CLASS;

public:
	virtual Ref< Layer > createInstance(Stage* stage, amalgam::IEnvironment* environment) const;

	virtual bool serialize(ISerializer& s);

private:
	friend class StagePipeline;

	resource::Id< video::Video > m_video;
	resource::Id< render::Shader > m_shader;
};

	}
}

#endif	// traktor_parade_VideoLayerData_H
