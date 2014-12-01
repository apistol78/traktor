#include "Core/Misc/String.h"
#include "Input/Binding/InConst.h"
#include "Input/Editor/InConstTraits.h"

namespace traktor
{
	namespace input
	{

std::wstring InConstTraits::getHeader(const IInputNode* node) const
{
	return L"Constant";
}

std::wstring InConstTraits::getDescription(const IInputNode* node) const
{
	const InConst* inConst = checked_type_cast< const InConst*, false >(node);
	return toString(inConst->m_value);
}

Ref< IInputNode > InConstTraits::createNode() const
{
	return new InConst();
}

void InConstTraits::getInputNodes(const IInputNode* node, std::map< const std::wstring, Ref< const IInputNode > >& outInputNodes) const
{
}

void InConstTraits::connectInputNode(IInputNode* node, const std::wstring& inputName, IInputNode* sourceNode) const
{
}

void InConstTraits::disconnectInputNode(IInputNode* node, const std::wstring& inputName) const
{
}

	}
}
