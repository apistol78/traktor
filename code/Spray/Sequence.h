#ifndef traktor_spray_Sequence_H
#define traktor_spray_Sequence_H

#include "Core/Serialization/ISerializable.h"

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
class T_DLLCLASS Sequence : public ISerializable
{
	T_RTTI_CLASS;

public:
	struct Key
	{
		float T;
		Ref< ITrigger > trigger;

		bool serialize(ISerializer& s);
	};

	bool bind(resource::IResourceManager* resourceManager);

	Ref< SequenceInstance > createInstance();

	virtual bool serialize(ISerializer& s);

	const std::vector< Key >& getKeys() const { return m_keys; }

private:
	std::vector< Key > m_keys;
};

	}
}

#endif	// traktor_spray_Sequence_H
