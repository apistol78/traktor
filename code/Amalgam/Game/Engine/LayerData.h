#pragma once

#include <string>
#include "Core/Ref.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AMALGAM_GAME_EXPORT)
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

/*! \brief
 * \ingroup Amalgam
 */
class T_DLLCLASS LayerData : public ISerializable
{
	T_RTTI_CLASS;

public:
	LayerData();

	virtual Ref< Layer > createInstance(Stage* stage, IEnvironment* environment) const = 0;

	virtual void serialize(ISerializer& s) override;

protected:
	friend class StagePipeline;

	std::wstring m_name;
	bool m_permitTransition;
};

	}
}
