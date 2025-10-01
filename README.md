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

## TODO
### High priority
- [ ] Unofficial opcodes

### Middle priority
- [ ] PPU scrolling
- [ ] Accurate CPU cycle timing (page cross penalties etc)
- [ ] Mappers other than 0

### Low priority
- [ ] APU emulation
- [ ] Expand and improve GUI
- [ ] PPU testing for accuracy
- [ ] Save states

## Menu controls

- **ESC** - Close menu/quit
- **F1** - ROM selection

### TODO
- **F2** - Control configuration  
- **F3** - Create save state
- **F4** - Load last save state
- **F5** - Save state selection
