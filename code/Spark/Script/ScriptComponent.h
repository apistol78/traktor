#ifndef traktor_spark_ScriptComponent_H
#define traktor_spark_ScriptComponent_H

#include "Core/Ref.h"
#include "Resource/Proxy.h"
#include "Spark/IComponent.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class IRuntimeClass;

	namespace spark
	{

class Sprite;

/*! \brief Script component instance.
 * \ingroup Spark
 */
class T_DLLCLASS ScriptComponent : public IComponent
{
	T_RTTI_CLASS;

public:
	ScriptComponent(Sprite* owner, const resource::Proxy< IRuntimeClass >& clazz);

	virtual void update();

	virtual void eventKey(wchar_t unicode);

	virtual void eventKeyDown(int32_t keyCode);

	virtual void eventKeyUp(int32_t keyCode);

	virtual void eventMouseDown(const Vector2& position, int32_t button);

	virtual void eventMouseUp(const Vector2& position, int32_t button);

	virtual void eventMousePress(const Vector2& position, int32_t button);

	virtual void eventMouseRelease(const Vector2& position, int32_t button);

	virtual void eventMouseMove(const Vector2& position, int32_t button);

	virtual void eventMouseEnter(const Vector2& position, int32_t button);

	virtual void eventMouseLeave(const Vector2& position, int32_t button);

	virtual void eventMouseWheel(const Vector2& position, int32_t delta);

	virtual void eventViewResize(int32_t width, int32_t height);

private:
	Sprite* m_owner;
	resource::Proxy< IRuntimeClass > m_class;
	Ref< ITypedObject > m_object;
	uint32_t m_methodUpdate;
	uint32_t m_methodEventKey;
	uint32_t m_methodEventKeyDown;
	uint32_t m_methodEventKeyUp;
	uint32_t m_methodEventMouseDown;
	uint32_t m_methodEventMouseUp;
	uint32_t m_methodEventMousePress;
	uint32_t m_methodEventMouseRelease;
	uint32_t m_methodEventMouseMove;
	uint32_t m_methodEventMouseEnter;
	uint32_t m_methodEventMouseLeave;
	uint32_t m_methodEventMouseWheel;
	uint32_t m_methodEventViewResize;
};

	}
}

#endif	// traktor_spark_ScriptComponent_H
