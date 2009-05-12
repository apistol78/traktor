#ifndef traktor_drawing_ImageInfo_H
#define traktor_drawing_ImageInfo_H

#include <string>
#include "Core/Heap/Ref.h"
#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DRAWING_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace drawing
	{

/*! \brief Image information.
 * \ingroup Drawing
 *
 * Additional information about the image.
 * Some image formats contain additional information about
 * author, copyright notes and such.
 */
class T_DLLCLASS ImageInfo : public Object
{
	T_RTTI_CLASS(ImageInfo)

public:
	void setAuthor(const std::wstring& author);

	const std::wstring& getAuthor() const;

	void setCopyright(const std::wstring& copyright);

	const std::wstring& getCopyright() const;

	void setFormat(const std::wstring& format);

	const std::wstring& getFormat() const;

private:
	std::wstring m_author;
	std::wstring m_copyright;
	std::wstring m_format;
};

	}
}

#endif	// traktor_drawing_ImageInfo_H
