#ifndef traktor_spray_SequenceData_H
#define traktor_spray_SequenceData_H

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

class ITriggerData;
class Sequence;

/*! \brief Trigger sequence persistent data.
 * \ingroup Spray
 */
class T_DLLCLASS SequenceData : public ISerializable
{
	T_RTTI_CLASS;

public:
	struct Key
	{
		float T;
		Ref< ITriggerData > trigger;

		void serialize(ISerializer& s);
	};

	Ref< Sequence > createSequence(resource::IResourceManager* resourceManager) const;

	virtual void serialize(ISerializer& s);

	const std::vector< Key >& getKeys() const { return m_keys; }

	 std::vector< Key >& getKeys() { return m_keys; }

private:
	std::vector< Key > m_keys;
};

	}
}

#endif	// traktor_spray_SequenceData_H
