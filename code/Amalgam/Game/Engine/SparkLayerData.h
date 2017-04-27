/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_amalgam_SparkLayerData_H
#define traktor_amalgam_SparkLayerData_H

#include "Amalgam/Game/Engine/LayerData.h"
#include "Core/Math/Color4ub.h"
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

class ImageProcessSettings;

	}

	namespace spark
	{

class SpriteData;

	}

	namespace amalgam
	{

/*! \brief Stage Spark layer persistent data.
 * \ingroup Amalgam
 */
class T_DLLCLASS SparkLayerData : public LayerData
{
	T_RTTI_CLASS;

public:
	SparkLayerData();

	virtual Ref< Layer > createInstance(Stage* stage, IEnvironment* environment) const T_OVERRIDE T_FINAL;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	friend class StagePipeline;

	resource::Id< spark::SpriteData > m_sprite;
	resource::Id< render::ImageProcessSettings > m_imageProcess;
	Color4ub m_background;
};

	}
}

#endif	// traktor_amalgam_SparkLayerData_H
