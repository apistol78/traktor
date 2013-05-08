#ifndef traktor_editor_AgentStatus_H
#define traktor_editor_AgentStatus_H

#include "Core/Guid.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class PropertyGroup;

	namespace editor
	{

/*! \brief
 * \ingroup Editor
 */
class T_DLLCLASS AgentStatus : public ISerializable
{
	T_RTTI_CLASS;

public:
	AgentStatus();

	AgentStatus(const Guid& buildGuid, bool result);

	const Guid& getBuildGuid() const;

	bool getResult() const;

	virtual void serialize(ISerializer& s);

private:
	Guid m_buildGuid;
	bool m_result;
};

	}
}

#endif	// traktor_editor_AgentStatus_H
