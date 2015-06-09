# Decrypt9 - Bootstrap-MOD

### Download

Check the releases tab. Extract and copy to the /3ds folder on your sd card.

## Generating xorpads for encrypted files

First build by running `BUILD.bat` in the project root.

### Decrypting gamecart dumps / extracted cia content.

Copy the Decrypt9 folder from `Decrypt9` with the .3dsx and .smdh after building to your 3ds sd card. The .3dsx and .smdh are a modified version of https://github.com/patois/Brahma with the Decrypt9 bin built in.

Then use `ncchinfo_gen.py` on your encrypted game (dump the game with the Gateway launcher).
Or if using on cia content extracted with ctrtool use `ncchinfo_tgen.py`, for example extracted theme pack cia's.

Then, **if you're on firmware that is less than 7.x**, create/edit `slot0x25KeyX.bin` in a **hex editor** and put in the 7.x KeyX (no, I won't give it to you).

Place `ncchinfo.bin` (and `slot0x25KeyX.bin`, if on less then 7.x) on your SD card, and run the decryptor. It should take a while, especially if you're decrypting a larger game.

Run from the homebrew menu with ninjhax.

## Credits

Roxas75 for the method of ARM9 code injection

Cha(N), Kane49, and all other FatFS contributors for FatFS

Normmatt for `sdmc.s` as well as project infrastructure (Makefile, linker setup, etc)

Relys, sbJFn5r for the decryptor
