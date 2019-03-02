#pragma once

#include "Input/Editor/InputNodeTraits.h"

namespace traktor
{
	namespace input
	{

class InGestureSwipeTraits : public InputNodeTraits
{
public:
	virtual std::wstring getHeader(const IInputNode* node) const override final;

	virtual std::wstring getDescription(const IInputNode* node) const override final;

	virtual Ref< IInputNode > createNode() const override final;

	virtual void getInputNodes(const IInputNode* node, std::map< const std::wstring, Ref< const IInputNode > >& outInputNodes) const override final;

	virtual void connectInputNode(IInputNode* node, const std::wstring& inputName, IInputNode* sourceNode) const override final;

	virtual void disconnectInputNode(IInputNode* node, const std::wstring& inputName) const override final;
};

	}
}

