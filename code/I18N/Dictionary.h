#ifndef traktor_i18n_Dictionary_H
#define traktor_i18n_Dictionary_H

#include <map>
#include <string>
#include "Core/Serialization/Serializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_I18N_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif 

namespace traktor
{
	namespace i18n
	{

class T_DLLCLASS Dictionary : public Serializable
{
	T_RTTI_CLASS(Dictionary)

public:
	bool has(const std::wstring& id) const;

	void set(const std::wstring& id, const std::wstring& text);

	bool get(const std::wstring& id, std::wstring& outText) const;

	const std::map< std::wstring, std::wstring >& get() const;

	virtual bool serialize(Serializer& s);

private:
	std::map< std::wstring, std::wstring > m_map;
};

	}
}

#endif	// traktor_i18n_Dictionary_H
