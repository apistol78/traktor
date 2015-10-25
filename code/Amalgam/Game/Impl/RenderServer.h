#ifndef traktor_amalgam_RenderServer_H
#define traktor_amalgam_RenderServer_H

#include "Amalgam/Game/IRenderServer.h"

namespace traktor
{

class PropertyGroup;

	namespace render
	{
	
class TextureFactory;

	}

	namespace amalgam
	{

class IEnvironment;

class RenderServer : public IRenderServer
{
	T_RTTI_CLASS;

public:
	enum UpdateResult
	{
		UrTerminate = 0,
		UrSuccess = 1,
		UrReconfigure = 2
	};

	RenderServer();

	virtual void destroy() = 0;

	virtual void createResourceFactories(IEnvironment* environment) = 0;

	virtual int32_t reconfigure(IEnvironment* environment, const PropertyGroup* settings) = 0;

	virtual UpdateResult update(PropertyGroup* settings);

	virtual int32_t getFrameRate() const T_OVERRIDE T_FINAL;

	virtual int32_t getThreadFrameQueueCount() const T_OVERRIDE T_FINAL;

	void setFrameRate(int32_t frameRate);

protected:
	Ref< render::IRenderSystem > m_renderSystem;
	Ref< render::IRenderView > m_renderView;
	Ref< render::TextureFactory > m_textureFactory;

private:
	int32_t m_frameRate;
};

	}
}

#endif	// traktor_amalgam_RenderServer_H
