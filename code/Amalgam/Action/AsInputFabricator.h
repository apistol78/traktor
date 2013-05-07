#ifndef traktor_amalgam_AsInputFabricator_H
#define traktor_amalgam_AsInputFabricator_H

#include "Amalgam/IEnvironment.h"
#include "Flash/Action/ActionObjectRelay.h"

namespace traktor
{
	namespace amalgam
	{

class AsInputFabricator : public flash::ActionObjectRelay
{
	T_RTTI_CLASS;

public:
	AsInputFabricator(amalgam::IEnvironment* environment);

	std::wstring getSourceDescription(const std::wstring& sourceId);

	bool fabricateSource(const std::wstring& sourceId, int32_t category, bool analogue);

	bool isFabricating();

	bool abortedFabricating();

	bool resetInputSource(const std::wstring& sourceId);

private:
	amalgam::IEnvironment* m_environment;
};

	}
}

#endif	// traktor_amalgam_AsInputFabricator_H
