#ifndef traktor_input_InputMappingAsset_H
#define traktor_input_InputMappingAsset_H

#include <list>
#include <map>
#include "Core/Guid.h"
#include "Core/RefSet.h"
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

class IInputNode;
class InputMappingSourceData;
class InputMappingStateData;

/*! \brief Input mapping asset.
 * \ingroup Input
 */
class T_DLLCLASS InputMappingAsset : public ISerializable
{
	T_RTTI_CLASS;

public:
	struct Position
	{
		int32_t x;
		int32_t y;
	};

	void addInputNode(IInputNode* inputNode);

	void removeInputNode(IInputNode* inputNode);

	const RefSet< IInputNode >& getInputNodes() const;

	void setPosition(const Object* object, const Position& position);

	Position getPosition(const Object* object) const;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

	void setSourceData(InputMappingSourceData* sourceData) { m_sourceData = sourceData; }

	InputMappingSourceData* getSourceData() { return m_sourceData; }

	const InputMappingSourceData* getSourceData() const { return m_sourceData; }

	void setStateData(InputMappingStateData* stateData)  { m_stateData = stateData; }

	InputMappingStateData* getStateData() { return m_stateData; }

	const InputMappingStateData* getStateData() const { return m_stateData; }

	const std::list< Guid >& getDependencies() const { return m_dependencies; }

private:
	RefSet< IInputNode > m_inputNodes;
	std::map< Ref< const Object >, Position > m_positions;
	Ref< InputMappingSourceData > m_sourceData;
	Ref< InputMappingStateData > m_stateData;
	std::list< Guid > m_dependencies;
};

	}
}

#endif	// traktor_input_InputMappingAsset_H
