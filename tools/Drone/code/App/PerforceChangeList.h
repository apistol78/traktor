#ifndef traktor_drone_PerforceChangeList_H
#define traktor_drone_PerforceChangeList_H

#include <Core/Heap/Ref.h>
#include <Core/Serialization/Serializable.h>

namespace traktor
{
	namespace drone
	{

class PerforceChangeListFile;

class PerforceChangeList : public Serializable
{
	T_RTTI_CLASS(PerforceChangeList)

public:
	PerforceChangeList();

	PerforceChangeList(
		uint32_t change ,
		const std::wstring& user,
		const std::wstring& client,
		const std::wstring& description
	);

	uint32_t getChange() const;

	const std::wstring& getUser() const;

	const std::wstring& getClient() const;

	const std::wstring& getDescription() const;

	void setFiles(const RefArray< PerforceChangeListFile >& files);

	const RefArray< PerforceChangeListFile >& getFiles() const;

	virtual bool serialize(Serializer& s);

private:
	uint32_t m_change;
	std::wstring m_user;
	std::wstring m_client;
	std::wstring m_description;
	RefArray< PerforceChangeListFile > m_files;
};

	}
}

#endif	// traktor_drone_PerforceChangeList_H
