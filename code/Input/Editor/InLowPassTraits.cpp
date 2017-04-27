/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Misc/String.h"
#include "Input/Binding/InLowPass.h"
#include "Input/Editor/InLowPassTraits.h"

namespace traktor
{
	namespace input
	{

std::wstring InLowPassTraits::getHeader(const IInputNode* node) const
{
	return L"Low pass";
}

std::wstring InLowPassTraits::getDescription(const IInputNode* node) const
{
	return L"Low pass";
}

Ref< IInputNode > InLowPassTraits::createNode() const
{
	return new InLowPass();
}

void InLowPassTraits::getInputNodes(const IInputNode* node, std::map< const std::wstring, Ref< const IInputNode > >& outInputNodes) const
{
	const InLowPass* ilp = checked_type_cast< const InLowPass*, false >(node);
	outInputNodes[L"Input"] = ilp->m_source;
	outInputNodes[L"Coeff"] = ilp->m_coeff;
}

void InLowPassTraits::connectInputNode(IInputNode* node, const std::wstring& inputName, IInputNode* sourceNode) const
{
	InLowPass* ilp = checked_type_cast< InLowPass*, false >(node);
	if (inputName == L"Input")
		ilp->m_source = sourceNode;
	else
		ilp->m_coeff = sourceNode;
}

void InLowPassTraits::disconnectInputNode(IInputNode* node, const std::wstring& inputName) const
{
	InLowPass* ilp = checked_type_cast< InLowPass*, false >(node);
	if (inputName == L"Input")
		ilp->m_source = 0;
	else
		ilp->m_coeff = 0;
}

	}
}
