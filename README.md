# Patcher-VS

A game client updater for Metin2 servers, written in C++ with Qt 5.

When it starts, the patcher downloads a file list from your patch server, checks the local client against it with SHA-256, downloads only what is missing or changed, and then lets the player start the game.

<!-- Add a screenshot here, e.g. ![Patcher-VS](docs/screenshot.png) -->

## Features

- Incremental updates: only missing or changed files are downloaded.
- SHA-256 check on a worker thread, so the window stays responsive while a large client is hashed.
- Folder list from the server, created before any download starts.
- Per-folder exceptions for files players may change (music, for example): they are downloaded when missing and never overwritten.
- Frameless, draggable window with a replaceable skin (the background and buttons are PNGs compiled into the executable).
- Current file name, progress bar and download speed while patching.
- **Start** launches the game client and closes the patcher; **Settings** opens the client's config tool.
- Builds into a single executable (static Qt, OpenSSL and C runtime), so there are no DLLs to ship with it.

## How it works

1. Downloads `patchlist.txt`, `folders.txt` and `exceptions.txt` from the server root into the current folder.
2. Creates every folder listed in `folders.txt`.
3. Goes through `patchlist.txt` and queues each file that is missing locally or whose SHA-256 differs. Files covered by `exceptions.txt` are queued only when missing.
4. Downloads the queue one file at a time, then enables **Start**.

All paths are relative to the current folder, so the patcher has to sit in the client's root folder and be started from there. A shortcut needs the client folder as its *Start in* folder.

## Patch server

Any static web server works. Upload the client with its folder structure, plus the three list files, to the root of a (sub)domain:

```text
https://patch.example.com/
├── patchlist.txt
├── folders.txt
├── exceptions.txt
├── metin2client.exe
├── bgm/
│   └── login_window.mp3
└── pack/
    ├── locale.eix
    ├── locale.epk
    ├── root.eix
    └── root.epk
```

The patcher saves each file under its URL path, so the files have to be at the root of the host, not in a subfolder.

Sample lists are in [`examples/`](examples). All three files follow the same rules:

- plain text, one entry per line, values separated by spaces;
- paths relative to the client root, with forward slashes;
- no spaces in paths and no empty lines (reading stops at the first empty line);
- every file name needs an extension; the extension is everything after the first dot.

### `patchlist.txt`

Each line holds the file path, then its SHA-256 in lowercase hex. Anything after the hash is ignored.

```text
metin2client.exe 0b1c3f5e...
pack/root.eix 5d0f7a2c...
```

Leave out files players are expected to change, such as `metin2.cfg`, so the patcher never overwrites them.

### `folders.txt`

Every folder of the client, with a trailing slash. A file can only be saved if its folder is listed here.

```text
bgm/
pack/
```

### `exceptions.txt`

Each line holds a folder, written with the trailing slash as it appears in `patchlist.txt`, then the extensions to leave alone, separated by `|`. Those files are downloaded when they're missing, but after that they're never checked or overwritten. Files in the client root can't be excepted.

```text
bgm/ mp3|wav
```

### Generating the lists

Run this in PowerShell, inside the folder you upload:

```powershell
$root = (Get-Location).ProviderPath
$skip = 'patchlist.txt', 'folders.txt', 'exceptions.txt'   # add files players may change, e.g. 'metin2.cfg'

Get-ChildItem -Recurse -File |
    Where-Object { $skip -notcontains $_.Name } |
    ForEach-Object {
        $path = $_.FullName.Substring($root.Length + 1).Replace('\', '/')
        $hash = (Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256).Hash.ToLower()
        "$path $hash"
    } |
    Set-Content -Encoding ascii patchlist.txt

Get-ChildItem -Recurse -Directory |
    ForEach-Object { $_.FullName.Substring($root.Length + 1).Replace('\', '/') + '/' } |
    Set-Content -Encoding ascii folders.txt
```

Write `exceptions.txt` by hand.

## Configuration

Server-specific values are in [`config.h`](config.h):

| Setting | Default | Used for |
|---|---|---|
| `ServerUrl` | `https://patch.example.com/` | Patch server root, with the trailing slash |
| `WindowTitle` | `Your Server - Autopatcher` | Window and taskbar title |
| `GameExecutable` | `metin2client.exe` | Started by **Start** |
| `ConfigExecutable` | `config.exe` | Started by **Settings** |

The skin is made of these images in the project root, compiled in through `Resource.qrc`:

| File | Size | Used for |
|---|---|---|
| `background.png` | 694 × 489 | Window background, same size as the window in `mainwindow.ui` |
| `start.png`, `start-disabled.png` | 200 × 100 | **Start**, when enabled and while patching |
| `setari.png` | 150 × 75 | **Settings** (*setări* is Romanian for settings) |
| `app.ico` | 16 to 256 px | Executable and window icon |

Widget positions are in `mainwindow.ui` (Qt Designer). The executable's name is set by `TargetName` in `Patcher-VS.vcxproj`.

## Building

Requirements:

- Windows with Visual Studio 2019 or newer, the **Desktop development with C++** workload and the v142 toolset
- The Qt VS Tools extension
- Qt 5.15.2 built statically with the static runtime (`-static -static-runtime`); add `-openssl-linked` if the patch server uses HTTPS
- OpenSSL built as static libraries with `/MT` (`libcrypto.lib`, `libssl.lib`)

Steps:

1. In **Extensions → Qt VS Tools → Qt Versions**, add the static Qt build under the name `qt-5.15.2-static`. You can use another name if you also change `QtInstall` in `Patcher-VS.vcxproj`.
2. Put OpenSSL in `C:\openssl\STATIC\x64\Release`, with `include\` and `Lib\` inside. If it's somewhere else, set the `OpenSslDir` environment variable to that folder.
3. Open `Patcher-VS.sln`, choose **Release | x64** and build. The executable is written to `release\`.

The executable asks for administrator rights (`requireAdministrator`) so it can update clients installed under *Program Files*.

`Patcher-VS.pro` describes the same project for qmake and Qt Creator (`qmake OPENSSL_DIR=<path>`), but releases are built from the Visual Studio project.

## Security notes

- The file list and the files come from the same server, and the list isn't signed. The SHA-256 check catches corrupted or outdated files, but it doesn't protect against a compromised server or a tampered connection. Serve everything over HTTPS. Over plain `http://`, anyone on the network path can replace the client executable.
- Paths from the lists are used as they are (`..` and absolute paths aren't rejected), so only point the patcher at a server you control.
- Because the patcher runs as administrator, both of these risks are more serious.

## Project layout

| File | Role |
|---|---|
| `main.cpp` | Entry point |
| `mainwindow.*` | Frameless window, UI wiring, runs the update steps in order |
| `DownloadFile.*` | Downloads the three list files |
| `firstThread.*` | Worker thread: reads the lists, creates the folders, hashes local files and builds the download queue |
| `downloadManager.*` | Downloads the queue and reports progress and speed |
| `label_exit.*`, `label_minimize.*` | Clickable labels used as the close and minimize buttons |
| `config.h` | Server-specific settings |
| `Resource.qrc` | Images compiled into the executable |
| `patcher-vs_plugin_import.cpp` | Static Qt plugin imports (generated by qmake) |
| `examples/` | Sample list files |

## License

[MIT](LICENSE)

Metin2 is a trademark of its respective owner. This project isn't affiliated with or endorsed by the game's developer or publishers, and it doesn't include any game files.
