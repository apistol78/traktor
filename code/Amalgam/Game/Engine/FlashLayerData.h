#ifndef traktor_amalgam_FlashLayerData_H
#define traktor_amalgam_FlashLayerData_H

#include <map>
#include "Amalgam/Game/Engine/LayerData.h"
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
	namespace flash
	{

class FlashMovie;

	}

	namespace render
	{

class ImageProcessSettings;

	}

	namespace amalgam
	{

/*! \brief Stage Flash layer persistent data.
 * \ingroup Amalgam
 */
class T_DLLCLASS FlashLayerData : public LayerData
{
	T_RTTI_CLASS;

public:
	FlashLayerData();

	virtual Ref< Layer > createInstance(Stage* stage, IEnvironment* environment) const T_OVERRIDE T_FINAL;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	friend class StagePipeline;

	resource::Id< flash::FlashMovie > m_movie;
	std::map< std::wstring, resource::Id< flash::FlashMovie > > m_externalMovies;
	resource::Id< render::ImageProcessSettings > m_imageProcess;
	bool m_clearBackground;
	bool m_enableSound;
};

	}
}

#endif	// traktor_amalgam_FlashLayerData_H
