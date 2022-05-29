# easyrpg-savegame-manager-web

Small utility to manage/download EasyRPG savegames for the web Player


## getting started

```bash
git submodule update --init --recursive
```

## build native (linux)

```bash
cmake -B build-linux .
cmake --build build-linux
```

## build web (emscripten)

```bash
emcmake cmake -B build-web .
cmake --build build-web
```

For convenience there are some scripts in the root directory.

