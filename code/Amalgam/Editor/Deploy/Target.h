/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_amalgam_Target_H
#define traktor_amalgam_Target_H

#include <string>
#include "Core/RefArray.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AMALGAM_DEPLOY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace amalgam
	{

class TargetConfiguration;

/*! \brief
 * \ingroup Amalgam
 */
class T_DLLCLASS Target : public ISerializable
{
	T_RTTI_CLASS;

public:
	void setIdentifier(const std::wstring& identifier);

	const std::wstring& getIdentifier() const;

	void addConfiguration(TargetConfiguration* configuration);

	void removeConfiguration(TargetConfiguration* configuration);

	void removeAllConfigurations();

	const RefArray< TargetConfiguration >& getConfigurations() const;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	std::wstring m_identifier;
	RefArray< TargetConfiguration > m_configurations; 
};

	}
}

#endif	// traktor_amalgam_Target_H
