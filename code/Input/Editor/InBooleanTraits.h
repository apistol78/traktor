#ifndef traktor_input_InBooleanTraits_H
#define traktor_input_InBooleanTraits_H

#include "Input/Editor/InputNodeTraits.h"

namespace traktor
{
	namespace input
	{

class InBooleanTraits : public InputNodeTraits
{
public:
	virtual std::wstring getHeader(const IInputNode* node) const;

	virtual std::wstring getDescription(const IInputNode* node) const;

	virtual Ref< IInputNode > createNode() const;

	virtual void getInputNodes(const IInputNode* node, std::map< const std::wstring, Ref< const IInputNode > >& outInputNodes) const;

	virtual void connectInputNode(IInputNode* node, const std::wstring& inputName, IInputNode* sourceNode) const;

	virtual void disconnectInputNode(IInputNode* node, const std::wstring& inputName) const;
};

	}
}

#endif	// traktor_input_InBooleanTraits_H
