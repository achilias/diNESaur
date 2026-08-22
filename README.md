# DiNESaur

A simple NES emulator written in C++.

## Building

### Linux
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

## Menu controls

- **ESC** - Close menu/quit
- **F1** - ROM selection

## TODO
- [ ] Unofficial opcodes
- [ ] PPU scrolling
- [ ] Accurate CPU cycle timing (page cross penalties etc)
- [ ] Mappers other than 0
- [ ] APU emulation
- [ ] Expand and improve GUI
- [ ] PPU testing for accuracy
- [ ] Save states
- [ ] Configurable controls
