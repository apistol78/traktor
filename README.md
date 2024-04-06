# Traktor
Traktor is an open source C++ engine which has been used in a couple of commercial titles on Steam, PSN, iOS and Mac Store.

![image info](resources/documentation/images/screenshot%20-%20editor%20-%2003.png)

# Supported platforms
Currently Traktor is supported on Windows and Linux. Mobile versions (Android and iOS) are non-tested but kept up-to-date code wise.

# Prebuilt binaries
See [releases](https://github.com/apistol78/traktor/releases) for prebuilt binaries. However the releases should not be expected to be latest and therefor we recommend building the engine from source for latest bug fixes and features.

# How to build
[Linux](resources/documentation/markdown/Build%20Linux.md)

[Windows](resources/documentation/markdown/Build%20Windows.md)

# Features

## Editor

- Feature rich editor and tools, editor first class priority.
- One-click deploy, run and debug on any connected target.
- Active connection and hot reloading of assets to all launched games regardless of target platform.
- Clean separation of editor code, easy to add new editors etc.
- Undo/redo, localization, themes etc.
- Formalized pipeline/flow to cook assets to runtime optimized data; highly parallized and easy to add new pipelines.

## Rendering

- Advanced frame graph based renderer using Vulkan.
- GPU based occlusion culling (work in progress).
- Graph based shader editor with possibility to inline shader code if necessary.
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