# Traktor

## How to build

1. Ensure system are up to date for building Traktor.
```
sudo ./scripts/misc/install-linux-deps.sh
```

2. Ensure all 3rdp dependencies are installed or updated in the 3rdp directory.
```
./bin/linux/releasestatic/Traktor.Run.App ./scripts/misc/update-3rdp.run
```

3. Generate project files.
```
./scripts/build-projects-make-linux.sh
```

4. Build (using vscode integration script manually)
```
./scripts/build-vscode.sh Linux
```
