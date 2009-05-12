#ifndef traktor_editor_Asset_H
#define traktor_editor_Asset_H

#include "Core/Serialization/Serializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace editor
	{

/*! \brief Base class for assets.
 *
 * Derive from this class for assets which reference
 * source assets such as image files etc.
 */
class T_DLLCLASS Asset : public Serializable
{
	T_RTTI_CLASS(Asset)

public:
	Asset(const std::wstring& fileName = L"");

	void setFileName(const std::wstring& fileName);

	const std::wstring& getFileName() const;

	virtual bool serialize(Serializer& s);

	virtual const Type* getOutputType() const = 0;

private:
	std::wstring m_fileName;
};

	}
}

#endif	// traktor_editor_Asset_H
