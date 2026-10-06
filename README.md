# DiNESaur

A simple NES emulator written in C.

Able to run games like Donkey Kong and Balloon Fight.

### Note
This is a learning project, and therefore developed almost entirely without AI assistance (only used for code review and speeding up repetitive edits).

## Building

### MacOS / Linux
```sh
git clone --recurse-submodules https://github.com/MariosAchilias/diNESaur.git
cd diNESaur
cmake -S . -B build/
make -C build/
```
Run ``./build/dinesaur``
### Windows
```powershell
git clone --recurse-submodules https://github.com/MariosAchilias/diNESaur.git
cd diNESaur
cmake -S . -B build/
```
Open ``build/dinesaur.sln`` in Visual Studio and build target ``dinesaur``.

## TODO
- [ ] Unofficial opcodes
- [ ] Accurate CPU cycle timing (page cross penalties etc)
- [ ] Mappers other than 0
- [ ] APU emulation
- [ ] PPU testsuite
- [ ] PPU scrolling
