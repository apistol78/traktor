#ifndef traktor_amalgam_Layer_H
#define traktor_amalgam_Layer_H

#include "Core/Object.h"
#include "Render/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AMALGAM_EXPORT)
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

	namespace amalgam
	{

class Stage;

class T_DLLCLASS Layer : public Object
{
	T_RTTI_CLASS;

public:
	Layer(
		Stage* stage,
		const std::wstring& name
	);

	virtual ~Layer();

	void destroy();

	virtual void transition(Layer* fromLayer) = 0;

	virtual void prepare() = 0;

	virtual void update(amalgam::IUpdateControl& control, const amalgam::IUpdateInfo& info) = 0;

	virtual void build(const amalgam::IUpdateInfo& info, uint32_t frame) = 0;

	virtual void render(render::EyeType eye, uint32_t frame) = 0;

	virtual void preReconfigured() = 0;

	virtual void postReconfigured() = 0;

	Stage* getStage() const { return m_stage; }

	const std::wstring& getName() const { return m_name; }

private:
	Stage* m_stage;
	std::wstring m_name;
};

	}
}

#endif	// traktor_amalgam_Layer_H
