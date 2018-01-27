/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_spray_Sequence_H
#define traktor_spray_Sequence_H

#include <vector>
#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

class IEntityEvent;

	}

	namespace spray
	{

class SequenceInstance;

/*! \brief Trigger sequence.
 * \ingroup Spray
 */
class T_DLLCLASS Sequence : public Object
{
	T_RTTI_CLASS;

public:
	struct Key
	{
		float T;
		Ref< world::IEntityEvent > event;
	};

	Sequence(const std::vector< Key >& keys);

	Ref< SequenceInstance > createInstance() const;

private:
	friend class SequenceInstance;

	std::vector< Key > m_keys;
};

	}
}

#endif	// traktor_spray_Sequence_H
