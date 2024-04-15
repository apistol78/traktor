
---
1. Ensure all 3rdp dependencies are installed or updated in the 3rdp directory.

**Note! This will download and install Vulkan SDK automatically!**

```
.\script\update-3rdp.bat
```

---
2. Generate solution.
```
.\scripts\build-projects-vs2022-win64.bat
```

---
3. Open generated solution in Visual Studio.
```
.\scripts\open-projects-win64.bat
```

---
4. Build solution and launch editor.

Ensure **Traktor.Editor.App** is set as the startup application.
