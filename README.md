# Traktor
Traktor is an open-source 3d game engine written in C++, it has powered several successful commercial titles across various platforms including Steam, PSN, iOS, and the Mac Store.

![image info](resources/documentation/images/screenshot%20-%20editor%20-%2004.png)

# Why?
Traktor isn't just another game engine; it's a meticulously crafted toolkit designed for optimal performance and flexibility. Here's why it stands out:

* **Lean and Well-Designed:** Each module boasts clean responsibilities, ensuring an elegant and efficient codebase.

* **Minimal Footprint:** Traktor prioritizes low memory and storage usage, ensuring streamlined performance.

* **Optimized for Efficiency:** Utilizing modern rendering techniques and efficient algorithms, Traktor delivers exceptional performance.

* **Tailored Flexibility:** Unlike bloated, one-size-fits-all engines, Traktor is customizable to fit the unique needs of every project.

# Supported platforms
Traktor currently offers full support for Windows and Linux platforms. While mobile versions for Android and iOS are available, they are not officially tested but are regularly updated to maintain compatibility.

# Prebuilt binaries
See [releases](https://github.com/apistol78/traktor/releases) for prebuilt binaries. However the releases should not be expected to be latest and therefor we recommend building the engine from source for latest bug fixes and features.

# How to build
**Linux:** Refer to [Build Linux](resources/documentation/markdown/Build%20Linux.md) for detailed instructions.

**Windows:** Follow the guidelines outlined in [Build Windows](resources/documentation/markdown/Build%20Windows.md) for seamless setup.

# Features

## Editor

- Feature rich editor and tools, editor first class priority.
- One-click deploy, run and debug on any connected target.
- Active connection and hot reloading of assets to all launched games regardless of target platform.
- Clean separation of editor code, easy to add new editors etc.
- Undo/redo, localization, themes etc.
- Formalized pipeline/flow to cook assets to runtime optimized data; highly parallized and easy to add new pipelines.
- Light and dark themes, easily customizable.

## Rendering

- Advanced frame graph based renderer using Vulkan.
- GPU based occlusion culling (work in progress).
- Graph based shader editor with possibility to inline shader code when necessary.
- Tile based deferred and forward+ rendering paths.

## Scripting
- LUA as primary scripting language.
- Integrated debugger and profiler.

## Sound
- Full high definition pipeline supporting any number of channels (2.0, 2.1, 5.1, 7.1 etc).
- Supports many different backends (XAudio2, DirectSound, WinMM, OpenAL, OpenSL, ALSA, Pulse etc).
- User graph based filters.
- Sound banks for easy customization of sound effects.
- MP3, Flac, OGG streaming.

## Physics
- Integrated Bullet physics.
