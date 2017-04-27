/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_amalgam_AsSoundDriver_H
#define traktor_amalgam_AsSoundDriver_H

#include "Core/RefArray.h"
#include "Flash/Action/ActionObjectRelay.h"

namespace traktor
{
	namespace amalgam
	{

/*! \brief
 * \ingroup Amalgam
 */
class AsSoundDriver : public flash::ActionObjectRelay
{
	T_RTTI_CLASS;

public:
	AsSoundDriver();

	AsSoundDriver(const TypeInfo* soundDriverType);

	static RefArray< AsSoundDriver > getAvailable();

	std::wstring getName() const;

private:
	const TypeInfo* m_soundDriverType;
};

	}
}

#endif	// traktor_amalgam_AsSoundDriver_H
