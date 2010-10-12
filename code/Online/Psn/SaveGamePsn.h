#ifndef traktor_online_SaveGamePsn_H
#define traktor_online_SaveGamePsn_H

#include "Core/Ref.h"
#include "Online/ISaveGame.h"

namespace traktor
{
	namespace online
	{

class SaveGamePsn : public ISaveGame
{
	T_RTTI_CLASS;

public:
	SaveGamePsn(const std::wstring& name, ISerializable* attachment);

	virtual std::wstring getName() const;

	virtual Ref< ISerializable > getAttachment() const;

private:
	std::wstring m_name;
	Ref< ISerializable > m_attachment;
};

	}
}

#endif	// traktor_online_SaveGamePsn_H
