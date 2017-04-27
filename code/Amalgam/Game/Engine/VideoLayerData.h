/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_amalgam_VideoLayerData_H
#define traktor_amalgam_VideoLayerData_H

#include "Amalgam/Game/Engine/LayerData.h"
#include "Core/Math/Aabb2.h"
#include "Resource/Id.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AMALGAM_GAME_EXPORT)
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

/*! \brief
 * \ingroup Amalgam
 */
class T_DLLCLASS VideoLayerData : public LayerData
{
	T_RTTI_CLASS;

public:
	VideoLayerData();

	virtual Ref< Layer > createInstance(Stage* stage, IEnvironment* environment) const T_OVERRIDE T_FINAL;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	friend class StagePipeline;

	resource::Id< video::Video > m_video;
	resource::Id< render::Shader > m_shader;
	Aabb2 m_screenBounds;
	bool m_visible;
	bool m_autoPlay;
	bool m_repeat;
};

	}
}

#endif	// traktor_amalgam_VideoLayerData_H
