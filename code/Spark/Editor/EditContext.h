#ifndef traktor_spark_EditContext_H
#define traktor_spark_EditContext_H

#include "Core/Object.h"
#include "Core/Math/Vector2.h"

namespace traktor
{
	namespace spark
	{

class CharacterAdapter;
class Context;
class Sprite;

class EditContext : public Object
{
	T_RTTI_CLASS;

public:
	EditContext(Context* context);

	bool setSprite(Sprite* sprite);

	CharacterAdapter* hitTest(const Vector2& position) const;

	CharacterAdapter* getRoot();

	const RefArray< CharacterAdapter >& getAdapters() const;

	RefArray< CharacterAdapter > getSelectedAdapters() const;

	void setGridSpacing(int32_t gridSpacing);

	int32_t getGridSpacing() const;

	Context* getContext() const;

private:
	Ref< Context > m_context;
	Ref< CharacterAdapter > m_root;
	RefArray< CharacterAdapter > m_adapters;
	int32_t m_gridSpacing;
};

	}
}

#endif	// traktor_spark_EditContext_H
