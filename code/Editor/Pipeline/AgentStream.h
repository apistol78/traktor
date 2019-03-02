#pragma once

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
class T_DLLCLASS AgentStream : public ISerializable
{
	T_RTTI_CLASS;

public:
	AgentStream();

	AgentStream(uint32_t publicId);

	uint32_t getPublicId() const;

	virtual void serialize(ISerializer& s) override final;

private:
	uint32_t m_publicId;
};

	}
}

