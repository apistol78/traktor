#ifndef traktor_editor_AgentConnect_H
#define traktor_editor_AgentConnect_H

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
class T_DLLCLASS AgentConnect : public ISerializable
{
	T_RTTI_CLASS;

public:
	AgentConnect();

	AgentConnect(
		const Guid& sessionId,
		const PropertyGroup* settings,
		const std::wstring& host,
		uint16_t databasePort,
		uint16_t streamServerPort
	);

	const Guid& getSessionId() const;

	const PropertyGroup* getSettings() const;

	const std::wstring& getHost() const;

	uint16_t getDatabasePort() const;

	uint16_t getStreamServerPort() const;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	Guid m_sessionId;
	Ref< const PropertyGroup > m_settings;
	std::wstring m_host;
	uint16_t m_databasePort;
	uint16_t m_streamServerPort;
};

	}
}

#endif	// traktor_editor_AgentConnect_H
