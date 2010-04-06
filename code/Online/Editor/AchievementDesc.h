#ifndef traktor_online_AchievementDesc_H
#define traktor_online_AchievementDesc_H

#include "Core/Io/Path.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ONLINE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace online
	{

class T_DLLCLASS AchievementDesc : public ISerializable
{
	T_RTTI_CLASS;

public:
	AchievementDesc();

	void setId(const std::wstring& id);

	const std::wstring& getId() const;

	void setImagePath(const Path& imagePath);

	const Path& getImagePath() const;

	void setName(const std::wstring& name);

	const std::wstring& getName() const;

	void setDescription(const std::wstring& description);

	const std::wstring& getDescription() const;

	void setHidden(bool hidden);

	bool getHidden() const;

	virtual bool serialize(ISerializer& s);

private:
	std::wstring m_id;
	Path m_imagePath;
	std::wstring m_name;
	std::wstring m_description;
	bool m_hidden;
};

	}
}

#endif	// traktor_online_AchievementDesc_H
