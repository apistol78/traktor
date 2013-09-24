#ifndef traktor_amalgam_FlashLayerData_H
#define traktor_amalgam_FlashLayerData_H

#include "Amalgam/LayerData.h"
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
	namespace flash
	{

class FlashMovie;

	}

	namespace amalgam
	{

class T_DLLCLASS FlashLayerData : public LayerData
{
	T_RTTI_CLASS;

public:
	FlashLayerData();

	virtual Ref< Layer > createInstance(Stage* stage, amalgam::IEnvironment* environment) const;

	virtual void serialize(ISerializer& s);

private:
	friend class StagePipeline;

	resource::Id< flash::FlashMovie > m_movie;
	std::map< std::wstring, resource::Id< flash::FlashMovie > > m_externalMovies;
	bool m_clearBackground;
	bool m_enableSound;
};

	}
}

#endif	// traktor_amalgam_FlashLayerData_H
