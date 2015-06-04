#include "Amalgam/Game/Engine/Action/AsInputFabricator.h"
#include "Input/Binding/IInputSource.h"
#include "Input/Binding/InputMapping.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.AsInputFabricator", AsInputFabricator, flash::ActionObjectRelay)

AsInputFabricator::AsInputFabricator(amalgam::IEnvironment* environment)
:	flash::ActionObjectRelay("traktor.amalgam.InputFabricator")
,	m_environment(environment)
{
}

std::wstring AsInputFabricator::getSourceDescription(const std::wstring& sourceId)
{
	input::IInputSource* source = m_environment->getInput()->getInputMapping()->getSource(sourceId);
	return source ? source->getDescription() : L"";
}

bool AsInputFabricator::fabricateSource(const std::wstring& sourceId, int32_t category, bool analogue)
{
	return m_environment->getInput()->fabricateInputSource(sourceId, (input::InputCategory)category, analogue);
}

bool AsInputFabricator::isFabricating()
{
	return m_environment->getInput()->isFabricating();
}

bool AsInputFabricator::abortedFabricating()
{
	return m_environment->getInput()->abortedFabricating();
}

bool AsInputFabricator::resetInputSource(const std::wstring& sourceId)
{
	return m_environment->getInput()->resetInputSource(sourceId);
}

void AsInputFabricator::apply()
{
	m_environment->getInput()->apply();
}

void AsInputFabricator::revert()
{
	m_environment->getInput()->revert();
}

	}
}
