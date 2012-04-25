#ifndef traktor_spray_Sequence_H
#define traktor_spray_Sequence_H

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
	namespace resource
	{

class IResourceManager;

	}

	namespace spray
	{

class ITrigger;
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
		Ref< ITrigger > trigger;
	};

	Sequence(const std::vector< Key >& keys);

	Ref< SequenceInstance > createInstance() const;

private:
	std::vector< Key > m_keys;
};

	}
}

#endif	// traktor_spray_Sequence_H
