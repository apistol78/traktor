#ifndef traktor_parade_Layer_H
#define traktor_parade_Layer_H

#include "Core/Object.h"
#include "Render/Types.h"
#include "Resource/Proxy.h"
#include "Script/Any.h"

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
		Stage* stage,
		const std::wstring& name,
		const resource::Proxy< script::IScriptContext >& scriptContext
	);

	virtual ~Layer();

	void destroy();

	virtual void prepare() = 0;

	virtual void update(amalgam::IUpdateControl& control, const amalgam::IUpdateInfo& info) = 0;

	virtual void build(const amalgam::IUpdateInfo& info, uint32_t frame) = 0;

	virtual void render(render::EyeType eye, uint32_t frame) = 0;

	virtual void leave() = 0;

	virtual void reconfigured() = 0;

	Stage* getStage() const { return m_stage; }

	const std::wstring& getName() const { return m_name; }

protected:
	void flushScript();

	bool validateScriptContext();

	script::Any invokeScriptUpdate(amalgam::IUpdateControl& control, const amalgam::IUpdateInfo& info);

	script::Any invokeScriptMethod(const std::wstring& method, uint32_t argc, const script::Any* argv);

	bool isInitialized() const { return m_initialized; }

private:
	Stage* m_stage;
	std::wstring m_name;
	resource::Proxy< script::IScriptContext > m_scriptContext;
	bool m_initialized;
};

	}
}

#endif	// traktor_parade_Layer_H
