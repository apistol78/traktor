#ifndef traktor_parade_FlashLayerData_H
#define traktor_parade_FlashLayerData_H

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
	namespace flash
	{

class FlashMovie;

	}

	namespace parade
	{

class T_DLLCLASS FlashLayerData : public LayerData
{
	T_RTTI_CLASS;

public:
	FlashLayerData();

	virtual Ref< Layer > createInstance(amalgam::IEnvironment* environment) const;

	virtual bool serialize(ISerializer& s);

private:
	friend class StagePipeline;

	resource::Id< flash::FlashMovie > m_movie;
	bool m_clearBackground;
};

	}
}

#endif	// traktor_parade_FlashLayerData_H
