/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_input_InGestureTapTraits_H
#define traktor_input_InGestureTapTraits_H

#include "Input/Editor/InputNodeTraits.h"

namespace traktor
{
	namespace input
	{

class InGestureTapTraits : public InputNodeTraits
{
public:
	virtual std::wstring getHeader(const IInputNode* node) const T_OVERRIDE T_FINAL;

	virtual std::wstring getDescription(const IInputNode* node) const T_OVERRIDE T_FINAL;

	virtual Ref< IInputNode > createNode() const T_OVERRIDE T_FINAL;

	virtual void getInputNodes(const IInputNode* node, std::map< const std::wstring, Ref< const IInputNode > >& outInputNodes) const T_OVERRIDE T_FINAL;

	virtual void connectInputNode(IInputNode* node, const std::wstring& inputName, IInputNode* sourceNode) const T_OVERRIDE T_FINAL;

	virtual void disconnectInputNode(IInputNode* node, const std::wstring& inputName) const T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_input_InGestureTapTraits_H
