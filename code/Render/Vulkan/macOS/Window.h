#pragma once

#include "Core/Object.h"
#include "Render/Types.h"

namespace traktor
{
	namespace render
	{

class Window : public Object
{
public:
    virtual ~Window();

	bool create(int32_t width, int32_t height);

	void setTitle(const wchar_t* title);

	void setFullScreenStyle(int32_t width, int32_t height);

	void setWindowedStyle(int32_t width, int32_t height);

	void showCursor();

	void hideCursor();

	void show();

	void center();

	bool update(RenderEvent& outEvent);

	bool isFullScreen() const;

	bool isActive() const;

    void* getView() const;

private:
    void* m_window = nullptr;
};

	}
}
