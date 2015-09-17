#ifndef traktor_spark_TextInstance_H
#define traktor_spark_TextInstance_H

#include "Resource/Proxy.h"
#include "Spark/CharacterInstance.h"
#include "Spark/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace resource
	{

class IResourceManager;

	}

	namespace spark
	{

class Font;
class Text;

/*! \brief Text character instance.
 * \ingroup Spark
 */
class T_DLLCLASS TextInstance : public CharacterInstance
{
	T_RTTI_CLASS;

public:
	TextInstance(const CharacterInstance* parent);

	void setText(const std::wstring& text);

	const std::wstring& getText() const;

	void setHeight(float height);

	float getHeight() const;

	void setBounds(const Aabb2& bounds);

	void setHorizontalAlign(Align horizontalAlign);

	Align getHorizontalAlign() const;

	void setVerticalAlign(Align verticalAlign);

	Align getVerticalAlign() const;

	virtual Aabb2 getBounds() const;

	virtual void render(render::RenderContext* renderContext) const T_FINAL;

private:
	friend class Text;

	resource::Proxy< Font > m_font;
	std::wstring m_text;
	float m_height;
	Aabb2 m_bounds;
	Align m_horizontalAlign;
	Align m_verticalAlign;
};

	}
}

#endif	// traktor_spark_TextInstance_H
