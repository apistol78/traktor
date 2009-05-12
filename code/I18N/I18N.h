#ifndef traktor_i18n_I18N_H
#define traktor_i18n_I18N_H

#include "Core/Heap/Ref.h"
#include "Core/Singleton/Singleton.h"

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

class Dictionary;

/*! \brief Internationalization manager. */
class T_DLLCLASS I18N : public Singleton
{
	T_RTTI_CLASS(I18N)

public:
	static I18N& getInstance();

	void appendDictionary(Dictionary* dictionary, bool overrideExisting = false);

	Dictionary* getMasterDictionary();

	std::wstring get(const std::wstring& id, const std::wstring& defaultText = L"") const;

protected:
	virtual void destroy();

private:
	Ref< Dictionary > m_dictionary;

	I18N();
};

	}
}

#endif	// traktor_i18n_I18N_H
