#ifndef traktor_input_InputMappingAsset_H
#define traktor_input_InputMappingAsset_H

#include <list>
#include "Core/Guid.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_INPUT_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace input
	{

class InputMappingSourceData;
class InputMappingStateData;

/*! \brief Input mapping asset.
 * \ingroup Input
 */
class T_DLLCLASS InputMappingAsset : public ISerializable
{
	T_RTTI_CLASS;

public:
	virtual void serialize(ISerializer& s);

	const InputMappingSourceData* getSourceData() const { return m_sourceData; }

	const InputMappingStateData* getStateData() const { return m_stateData; }

	const std::list< Guid >& getDependencies() const { return m_dependencies; }

private:
	Ref< InputMappingSourceData > m_sourceData;
	Ref< InputMappingStateData > m_stateData;
	std::list< Guid > m_dependencies;
};

	}
}

#endif	// traktor_input_InputMappingAsset_H
