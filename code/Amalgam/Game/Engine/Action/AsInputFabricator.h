/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_amalgam_AsInputFabricator_H
#define traktor_amalgam_AsInputFabricator_H

#include "Amalgam/Game/IEnvironment.h"
#include "Flash/Action/ActionObjectRelay.h"

namespace traktor
{
	namespace amalgam
	{

/*! \brief
 * \ingroup Amalgam
 */
class AsInputFabricator : public flash::ActionObjectRelay
{
	T_RTTI_CLASS;

public:
	AsInputFabricator(IEnvironment* environment);

	std::wstring getSourceDescription(const std::wstring& sourceId);

	bool fabricateSource(const std::wstring& sourceId, int32_t category, bool analogue);

	bool isFabricating();

	bool abortedFabricating();

	bool resetInputSource(const std::wstring& sourceId);

	void apply();

	void revert();

private:
	IEnvironment* m_environment;
};

	}
}

#endif	// traktor_amalgam_AsInputFabricator_H
