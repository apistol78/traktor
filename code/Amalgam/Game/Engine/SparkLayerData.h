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

class Sprite;

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

	virtual Ref< Layer > createInstance(Stage* stage, IEnvironment* environment) const;

	virtual void serialize(ISerializer& s);

private:
	friend class StagePipeline;

	resource::Id< spark::Sprite > m_sprite;
	resource::Id< render::ImageProcessSettings > m_imageProcess;
	Color4ub m_background;
	int32_t m_width;
	int32_t m_height;
};

	}
}

#endif	// traktor_amalgam_SparkLayerData_H
