#pragma once

#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_INPUT_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace input
	{

class IInputNode;

/*! \brief
 * \ingroup Input
 */
class T_DLLCLASS InputStateData : public ISerializable
{
	T_RTTI_CLASS;

public:
	InputStateData();

	InputStateData(IInputNode* source);

	void setSource(IInputNode* source);

	const IInputNode* getSource() const;

	virtual void serialize(ISerializer& s) override final;

private:
	Ref< IInputNode > m_source;
};

	}
}

