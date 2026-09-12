# DiNESaur

A simple NES emulator written in C.

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
- [ ] PPU testing for accuracy
- [ ] PPU scrolling
