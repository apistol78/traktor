#ifndef traktor_parade_Stage_H
#define traktor_parade_Stage_H

#include <map>
#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Render/Types.h"

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

class IEnvironment;
class IStateManager;
class IUpdateInfo;

	}

	namespace parade
	{

class Layer;
class StageLoader;

class T_DLLCLASS Stage : public Object
{
	T_RTTI_CLASS;

public:
	Stage(
		amalgam::IEnvironment* environment,
		const std::map< std::wstring, Guid >& transitions,
		const Object* params
	);

	virtual ~Stage();

	void destroy();

	void addLayer(Layer* layer);

	void removeLayer(Layer* layer);

	void removeAllLayers();

	Layer* findLayer(const std::wstring& name) const;

	void terminate();

	Ref< Stage > loadStage(const std::wstring& name, const Object* params);

	Ref< StageLoader > loadStageAsync(const std::wstring& name, const Object* params);

	bool gotoStage(Stage* stage);

	bool update(amalgam::IStateManager* stateManager, const amalgam::IUpdateInfo& info);

	bool build(const amalgam::IUpdateInfo& info, uint32_t frame);

	void render(render::EyeType eye, uint32_t frame);

	void leave();

	void reconfigured();

	amalgam::IEnvironment* getEnvironment() { return m_environment; }

	const RefArray< Layer >& getLayers() const { return m_layers; }

	const Object* getParams() const { return m_params; }

private:
	Ref< amalgam::IEnvironment > m_environment;
	RefArray< Layer > m_layers;
	std::map< std::wstring, Guid > m_transitions;
	Ref< const Object > m_params;
	Ref< Stage > m_pendingStage;
	bool m_running;
};

	}
}

#endif	// traktor_parade_Stage_H
