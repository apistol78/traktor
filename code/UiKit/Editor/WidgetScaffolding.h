#ifndef traktor_uikit_WidgetScaffolding_H
#define traktor_uikit_WidgetScaffolding_H

#include "Core/Serialization/ISerializable.h"
#include "Resource/Id.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UIKIT_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class IRuntimeClass;

	namespace uikit
	{

class T_DLLCLASS WidgetScaffolding : public ISerializable
{
	T_RTTI_CLASS;

public:
	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

	const resource::Id< IRuntimeClass >& getScaffoldingClass() const { return m_scaffoldingClass; }

private:
	resource::Id< IRuntimeClass > m_scaffoldingClass;
};

	}
}

#endif	// traktor_uikit_WidgetScaffolding_H
