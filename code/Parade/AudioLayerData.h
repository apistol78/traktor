#ifndef traktor_parade_AudioLayerData_H
#define traktor_parade_AudioLayerData_H

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
	namespace parade
	{

class T_DLLCLASS AudioLayerData : public LayerData
{
	T_RTTI_CLASS;

public:
	virtual Ref< Layer > createInstance(amalgam::IEnvironment* environment) const;

	virtual bool serialize(ISerializer& s);
};

	}
}

#endif	// traktor_parade_AudioLayerData_H
