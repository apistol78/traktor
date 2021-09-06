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
	bool create(int32_t width, int32_t height);

	void setTitle(const wchar_t* title);

	void setFullScreenStyle(int32_t width, int32_t height);

	void setWindowedStyle(int32_t width, int32_t height);

	void showCursor();

	void hideCursor();

	void show();

	void center();

	bool update(RenderEvent& outEvent);

	// int32_t getWidth() const { return m_width; }

	// int32_t getHeight() const { return m_height; }

	// bool isFullScreen() const { return m_fullScreen; }

	// bool isActive() const { return m_active; }

    void* getView() const;

private:
    void* m_window = nullptr;
};

	}
}
