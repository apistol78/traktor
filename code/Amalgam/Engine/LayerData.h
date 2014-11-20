#ifndef traktor_amalgam_LayerData_H
#define traktor_amalgam_LayerData_H

#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AMALGAM_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace amalgam
	{

class IEnvironment;

	}

	namespace amalgam
	{

class Layer;
class Stage;

class T_DLLCLASS LayerData : public ISerializable
{
	T_RTTI_CLASS;

public:
	enum { Version = 3 };

	LayerData();

	virtual Ref< Layer > createInstance(Stage* stage, amalgam::IEnvironment* environment) const = 0;

	virtual void serialize(ISerializer& s);

protected:
	friend class StagePipeline;

	std::wstring m_name;
	bool m_permitTransition;
};

	}
}

#endif	// traktor_amalgam_LayerData_H
