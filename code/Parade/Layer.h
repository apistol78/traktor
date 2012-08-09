#ifndef traktor_parade_Layer_H
#define traktor_parade_Layer_H

#include "Core/Object.h"
#include "Render/Types.h"
#include "Resource/Proxy.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PARADE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace amalgam
	{

class IUpdateControl;
class IUpdateInfo;

	}

	namespace script
	{

class Any;
class IScriptContext;

	}

	namespace parade
	{

class Stage;

class T_DLLCLASS Layer : public Object
{
	T_RTTI_CLASS;

public:
	Layer(
		const std::wstring& name,
		const resource::Proxy< script::IScriptContext >& scriptContext
	);

	virtual ~Layer();

	void destroy();

	virtual void update(Stage* stage, amalgam::IUpdateControl& control, const amalgam::IUpdateInfo& info) = 0;

	virtual void build(Stage* stage, const amalgam::IUpdateInfo& info, uint32_t frame) = 0;

	virtual void render(Stage* stage, render::EyeType eye, uint32_t frame) = 0;

	virtual void leave(Stage* stage) = 0;

	virtual void reconfigured(Stage* stage) = 0;

	const std::wstring& getName() const { return m_name; }

protected:
	void flushScript();

	bool validateScriptContext(Stage* stage);

	void invokeScriptUpdate(Stage* stage, amalgam::IUpdateControl& control, const amalgam::IUpdateInfo& info);

	void invokeScriptMethod(Stage* stage, const std::wstring& method, uint32_t argc, const script::Any* argv);

	bool isInitialized() const { return m_initialized; }

private:
	std::wstring m_name;
	resource::Proxy< script::IScriptContext > m_scriptContext;
	bool m_initialized;
};

	}
}

#endif	// traktor_parade_Layer_H
