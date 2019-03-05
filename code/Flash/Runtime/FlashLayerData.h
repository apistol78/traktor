#pragma once

#include <map>
#include "Runtime/Engine/LayerData.h"
#include "Resource/Id.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_RUNTIME_EXPORT)
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

	namespace flash
	{

class Movie;

/*! \brief Stage Flash layer persistent data.
 * \ingroup Flash
 */
class T_DLLCLASS FlashLayerData : public runtime::LayerData
{
	T_RTTI_CLASS;

public:
	FlashLayerData();

	virtual Ref< runtime::Layer > createInstance(runtime::Stage* stage, runtime::IEnvironment* environment) const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	friend class FlashLayerPipeline;

	resource::Id< Movie > m_movie;
	std::map< std::wstring, resource::Id< Movie > > m_externalMovies;
	resource::Id< render::ImageProcessSettings > m_imageProcess;
	bool m_clearBackground;
	bool m_enableShapeCache;
	bool m_enableDirtyRegions;
	bool m_enableSound;
	uint32_t m_contextSize;
};

	}
}
