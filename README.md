# tagQ

Audio metadata tagger built with KDE6 frameworks and Qt6.

## Overview

**tagQ** is a feature-rich audio metadata editor inspired by TagScanner. It provides a comprehensive interface for editing audio file metadata (ID3 tags, Vorbis comments, etc.) with a modern KDE6 user interface.

## Features

### File Management
- Browse audio files in a tree-based folder structure
- Support for multiple audio formats (MP3, FLAC, OGG, M4A, WAV, AAC, Opus, WMA)
- Recursive folder scanning
- File filtering and search capabilities
- Drag & drop support

### Metadata Editing
- Edit common metadata fields:
  - Title, Artist, Album, Album Artist
  - Genre, Year, Track Number
  - Comments
- Batch editing capabilities
- Undo/Redo support
- Real-time preview
- Cover art management

### Audio Information Display
- File format detection
- Bitrate, sample rate, and duration information
- Audio properties viewer
- File size and codec information

### Advanced Features
- **Auto-tagging**: Automatically fill metadata from filename patterns
- **Batch Rename**: Rename files based on metadata templates
- **Template-based Tagging**: Apply patterns to multiple files
- **Cover Art**: Add and manage album artwork
- **Database Lookup**: Integration with online metadata sources (future feature)

### User Interface
- Split panel layout: folder browser, file list, metadata editor
- Sortable file list with customizable columns
- Keyboard shortcuts for common operations
- Context menus for quick actions
- Settings persistence

## Building

### Requirements
- CMake 3.16 or higher
- Qt6 (Core, Gui, Widgets, Concurrent)
- KDE Frameworks 6 (CoreAddons, WidgetsAddons, KIO, ConfigGui, Codecs)
- TagLib (for metadata reading/writing)

### Build Instructions

```bash
mkdir build
cd build
cmake ..
make
sudo make install
```

### Dependencies Installation

**Ubuntu/Debian:**
```bash
sudo apt install cmake qt6-base-dev qt6-tools-dev \
    libtag1-dev \
    extra-cmake-modules \
    libkf6coreaddons-dev libkf6widgetsaddons-dev libkf6kio-dev
```

**Fedora:**
```bash
sudo dnf install cmake qt6-qtbase-devel qt6-qttools-devel \
    taglib-devel \
    extra-cmake-modules \
    kf6-kcoreaddons-devel kf6-kwidgetsaddons-devel kf6-kio-devel
```

**Arch:**
```bash
sudo pacman -S cmake qt6-base qt6-tools \
    taglib \
    extra-cmake-modules \
    kf6-coreaddons kf6-widgetsaddons kf6-kio
```

## Project Structure

```
tagQ/
├── CMakeLists.txt              # CMake build configuration
├── src/
│   ├── main.cpp               # Application entry point
│   ├── application.h/cpp       # Application class
│   ├── mainwindow.h/cpp        # Main application window
│   ├── filebrowser.h/cpp       # Folder browser component
│   ├── filelist.h/cpp          # File list/table component
│   ├── metadataeditor.h/cpp    # Metadata editor component
│   ├── audiofile.h/cpp         # Audio file abstraction
│   ├── tagsmanager.h/cpp       # Tags operations manager
│   ├── resources.qrc           # Qt resource file
│   ├── tagQ.desktop            # Desktop entry
│   └── icons/                  # Application icons
└── README.md                   # This file
```

## Architecture

### Components

**AudioFile** - Wrapper around TagLib for reading/writing audio metadata
- Loads file properties (bitrate, sample rate, duration)
- Reads and writes common metadata tags
- Tracks modifications for batch operations

**FileBrowser** - Folder navigation interface
- Tree-based directory structure
- Quick access to common folders (Home, Music, etc.)
- Context menus for folder operations

**FileList** - Audio file display and management
- Table view of audio files with sortable columns
- Inline editing of metadata
- Multi-file selection
- Auto-tag and batch rename operations

**MetadataEditor** - Detailed metadata editing panel
- Individual field editors for all common tags
- File information display
- Cover art management
- Apply/Revert functionality

**TagsManager** - Batch operations and template processing
- Save multiple files
- Template-based auto-tagging
- Batch file renaming
- Pattern parsing and processing

### Signals and Slots

The components communicate via Qt signals/slots:
- FileBrowser → FileList: folderSelected(path)
- FileList → MetadataEditor: fileSelected(path)
- MetadataEditor → FileList: metadataChanged(path, tags)

## Usage

### Basic Workflow
1. Open the application
2. Browse to a folder containing audio files using the folder tree
3. Select files from the list
4. View and edit metadata in the right panel
5. Click "Apply" to save changes
6. Click "Save Tags" to write metadata to files

### Keyboard Shortcuts
- Ctrl+O: Open folder dialog
- Ctrl+Q: Exit application
- Ctrl+A: Select all files
- Ctrl+Shift+A: Deselect all files

### Auto-tagging
1. Select files in the list
2. Right-click and choose "Auto Tag from Filename"
3. Files with pattern "Artist - Title" will be auto-tagged

### Batch Rename
1. Select files to rename
2. Right-click and choose "Batch Rename"
3. Enter a pattern using {title}, {artist}, {album}, {track}
4. Files will be renamed accordingly

## Supported Audio Formats

- MP3 (ID3v1/v2)
- FLAC (Vorbis comments)
- OGG Vorbis
- M4A/AAC
- WAV
- Opus
- WMA

## Configuration

Settings are stored in `~/.config/kderc` in the `[tagQ]` group:
- Window geometry
- Window state (maximized, etc.)
- Recent folders
- User preferences

## Contributing

Contributions are welcome! Please feel free to submit issues and pull requests.

## License

GNU General Public License v3.0 - See LICENSE file for details

## Credits

Inspired by **TagScanner** by Dmitry Tretyakov - a powerful and feature-rich audio tag editor that inspired many of this application's design decisions.

## Roadmap

- [ ] Database lookup integration (MusicBrainz, Discogs)
- [ ] Multiple language support
- [ ] Plugins system
- [ ] Library management
- [ ] Playlist creation
- [ ] Smart folders/virtual collections
- [ ] Audio format conversion
- [ ] Spectral analysis
