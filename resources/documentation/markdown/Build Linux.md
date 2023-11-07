
---
1. Ensure system are up to date for building Traktor.

**This will install required apt packages!**

```
sudo ./scripts/misc/install-linux-deps.sh
```

---
2. Ensure all third-party dependencies are installed or updated in the 3rdp directory.
```
./bin/linux/releasestatic/Traktor.Run.App ./scripts/misc/update-3rdp.run
```

---
3. Generate project files.
```
./scripts/build-projects-make-linux.sh
```

---
4. Build (using vscode integration script manually)
```
./scripts/build-vscode.sh Linux
```

---
5. Launch editor
```
./scripts/run-editor.sh
```
