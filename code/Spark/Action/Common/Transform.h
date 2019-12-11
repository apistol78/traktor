#pragma once

#include "Spark/Action/ActionObjectRelay.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace spark
	{

class ColorTransform;
class CharacterInstance;

/*! Geometry transform wrapper.
 * \ingroup Spark
 */
class T_DLLCLASS Transform : public ActionObjectRelay
{
	T_RTTI_CLASS;

public:
	Transform(CharacterInstance* instance);

	Ref< ColorTransform > getColorTransform() const;

	void setColorTransform(const ColorTransform* colorTransform);

protected:
	virtual void trace(visitor_t visitor) const override final;

	virtual void dereference() override final;

private:
	Ref< CharacterInstance > m_instance;
};

	}
}

