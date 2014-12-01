#ifndef traktor_input_InputNodeTraits_H
#define traktor_input_InputNodeTraits_H

#include <map>
#include "Core/Object.h"

namespace traktor
{
	namespace input
	{

class IInputNode;

class InputNodeTraits : public Object
{
public:
	virtual std::wstring getHeader(const IInputNode* node) const = 0;

	virtual std::wstring getDescription(const IInputNode* node) const = 0;

	virtual void getInputNodes(const IInputNode* node, std::map< const std::wstring, Ref< const IInputNode > >& outInputNodes) const = 0;

	virtual void connectInputNode(IInputNode* node, const std::wstring& inputName, IInputNode* sourceNode) const = 0;

	virtual void disconnectInputNode(IInputNode* node, const std::wstring& inputName) const = 0;
};

	}
}

#endif	// traktor_input_InputNodeTraits_H
