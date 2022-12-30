# Wwise *.BNK File Extractor and Importer

This is a C++ rewrite and extension of **bnkextr** originally written by CTPAX-X in Delphi.
It extracts `WEM` files from the ever more popular Wwise `BNK` format.

Use [ww2ogg](https://github.com/hcs64/ww2ogg) to convert `WEM` files to the `OGG` format.

## Usage

```
Usage: bnkextr filename.bnk [/extract] [/import] [/swap] [/nodir] [/obj]
        /extract - extract the files to the folder
        /import - import the files from the folder
        /swap - swap byte order (use it for unpacking 'Army of Two')
        /nodir - create no additional directory for the *.wem files
        /obj - generate an objects.txt file with the extracted object data
```

## BNK Format

- See the [original Delphi code](bnkextr.dpr) for the initial file specification
- See the [XeNTaX wiki](https://wiki.xentax.com/index.php/Wwise_SoundBank_(*.bnk)) for a more complete file specification
- See the [bnk.bt](bnk.bt) file for [010 Editor](https://www.sweetscape.com/010editor/) specification
- See the [bnk.ksy](https://github.com/WolvenKit/wwise-audio-tools/blob/feat/bnk-tools/ksy/bnk.ksy) file by ADawesomeguy for a [Kaitai Struct](https://kaitai.io/) specification

### Supported HIRC Events

- Generic event type logging
- Event
- EventAction

## Build

### CMake

```
cmake -S . -B build/
cmake --build build/ --target install
```

### GCC

```
g++ bnkextr.cpp -std=c++17 -static -O2 -s -o bnkextr.exe
```

## License

- `bnkextr.dpr` falls under the original copryright holders rights and is solely kept for archival purpose
- `bnkextr.cpp` is available under 2 licenses: Public Domain or MIT -- choose whichever you prefer