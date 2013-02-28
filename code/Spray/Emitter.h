#ifndef traktor_spray_Emitter_H
#define traktor_spray_Emitter_H

#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Resource/Proxy.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class Shader;

	}

	namespace spray
	{

class EmitterInstance;
class Modifier;
class Source;

/*! \brief Emitter
 * \ingroup Spray
 */
class T_DLLCLASS Emitter : public Object
{
	T_RTTI_CLASS;

public:
	Emitter(
		Source* source,
		const RefArray< Modifier >& modifiers,
		const resource::Proxy< render::Shader >& shader,
		float middleAge,
		float cullNearDistance,
		float fadeNearRange,
		float warmUp,
		bool sort,
		bool worldSpace
	);

	Ref< EmitterInstance > createInstance(float duration) const;

	const Source* getSource() const { return m_source; }

	const RefArray< Modifier >& getModifiers() const { return m_modifiers; }

	const resource::Proxy< render::Shader >& getShader() const { return m_shader; }

	float getMiddleAge() const { return m_middleAge; }

	float getCullNearDistance() const { return m_cullNearDistance; }

	float getFadeNearRange() const { return m_fadeNearRange; }

	float getWarmUp() const { return m_warmUp; }

	bool getSort() const { return m_sort; }

	bool worldSpace() const { return m_worldSpace; }

private:
	Ref< Source > m_source;
	RefArray< Modifier > m_modifiers;
	resource::Proxy< render::Shader > m_shader;
	float m_middleAge;
	float m_cullNearDistance;
	float m_fadeNearRange;
	float m_warmUp;
	bool m_sort;
	bool m_worldSpace;
};

	}
}

#endif	// traktor_spray_Emitter_H
