#ifndef traktor_spark_TextInstance_H
#define traktor_spark_TextInstance_H

#include "Resource/Proxy.h"
#include "Spark/CharacterInstance.h"

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
class TextInstance : public CharacterInstance
{
	T_RTTI_CLASS;

public:
	TextInstance(const CharacterInstance* parent);

	void setText(const std::wstring& text);

	const std::wstring& getText() const;

	void setHeight(float height);

	float getHeight() const;

	virtual void render(render::RenderContext* renderContext) const T_FINAL;

private:
	friend class Text;

	resource::Proxy< Font > m_font;
	std::wstring m_text;
	float m_height;
};

	}
}

#endif	// traktor_spark_TextInstance_H
