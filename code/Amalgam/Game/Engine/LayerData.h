/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_amalgam_LayerData_H
#define traktor_amalgam_LayerData_H

#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AMALGAM_GAME_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace amalgam
	{

class IEnvironment;

	}

	namespace amalgam
	{

class Layer;
class Stage;

/*! \brief
 * \ingroup Amalgam
 */
class T_DLLCLASS LayerData : public ISerializable
{
	T_RTTI_CLASS;

public:
	enum { Version = 6 };

	LayerData();

	virtual Ref< Layer > createInstance(Stage* stage, IEnvironment* environment) const = 0;

	virtual void serialize(ISerializer& s) T_OVERRIDE;

protected:
	friend class StagePipeline;

	std::wstring m_name;
	bool m_permitTransition;
};

	}
}

#endif	// traktor_amalgam_LayerData_H
