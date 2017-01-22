#include "fs.h"
#include "draw.h"
#include "hid.h"
#include "platform.h"
#include "gamecart/protocol.h"
#include "gamecart/command_ctr.h"
#include "gamecart/command_ntr.h"
#include "gamecart/card_eeprom.h"
#include "decryptor/aes.h"
#include "decryptor/sha.h"
#include "decryptor/decryptor.h"
#include "decryptor/hashfile.h"
#include "decryptor/keys.h"
#include "decryptor/titlekey.h"
#include "decryptor/nandfat.h"
#include "decryptor/nand.h"
#include "decryptor/game.h"

#define CART_CHUNK_SIZE (u32) (1*1024*1024)


u32 GetSdCtr(u8* ctr, const char* path)
{
    // get AES counter, see: http://www.3dbrew.org/wiki/Extdata#Encryption
    // path is the part of the full path after //Nintendo 3DS/<ID0>/<ID1>
    u8 hashstr[256];
    u8 sha256sum[32];
    u32 plen = 0;
    // poor man's UTF-8 -> UTF-16
    for (u32 i = 0; i < 128; i++) {
        hashstr[2*i] = path[i];
        hashstr[2*i+1] = 0;
        if (path[i] == 0) {
            plen = i;
            break;
        }
    }
    sha_quick(sha256sum, hashstr, (plen + 1) * 2, SHA256_MODE);
    for (u32 i = 0; i < 16; i++)
        ctr[i] = sha256sum[i] ^ sha256sum[i+16];
    
    return 0;
}

u32 GetNcchCtr(u8* ctr, NcchHeader* ncch, u8 sub_id) {
    memset(ctr, 0x00, 16);
    if (ncch->version == 1) {
        memcpy(ctr, &(ncch->partitionId), 8);
        if (sub_id == 1) { // exHeader ctr
            add_ctr(ctr, 0x200); 
        } else if (sub_id == 2) { // exeFS ctr
            add_ctr(ctr, ncch->offset_exefs * 0x200);
        } else if (sub_id == 3) { // romFS ctr
            add_ctr(ctr, ncch->offset_romfs * 0x200);
        }
    } else {
        for (u32 i = 0; i < 8; i++)
            ctr[i] = ((u8*) &(ncch->partitionId))[7-i];
        ctr[8] = sub_id;
    }
    
    return 0;
}

u32 SdFolderSelector(char* path, u8* keyY, bool title_select)
{
    char** dirptr = (char**) 0x20400000; // allow using 0x8000 byte
    char* dirlist = (char*) 0x20408000; // allow using 0x80000 byte
    u32 n_dirs = 0;
    
    // the keyY is used to generate a proper base path
    // see here: https://www.3dbrew.org/wiki/Nand/private/movable.sed
    u32 sha256sum[8];
    char base_path[64];
    sha_quick(sha256sum, keyY, 16, SHA256_MODE);
    snprintf(base_path, 63, "/Nintendo 3DS/%08X%08X%08X%08X",
        (unsigned int) sha256sum[0], (unsigned int) sha256sum[1],
        (unsigned int) sha256sum[2], (unsigned int) sha256sum[3]);
    Debug("<id0> is %s", base_path + 14);
    if (!GetFileList(base_path, dirlist, 0x80000, true, false, true)) {
        Debug("Failed retrieving the dirlist");
        return 1;
    }
    
    // parse the dirlist for usable entries
    for (char* dir = strtok(dirlist, "\n"); dir != NULL; dir = strtok(NULL, "\n")) {
        if (strnlen(dir, 256) <= 13 + 33 + 33)
            continue;
        if (strchrcount(dir, '/') > 6)
            continue; // allow a maximum depth of 6 for the full folder
        char* subdir = dir + 13 + 33 + 33; // length of ("/Nintendo 3DS" + "/<id0>" + "/<id1>");
        if ((strncmp(subdir, "/dbs", 4) != 0) && (strncmp(subdir, "/extdata", 8) != 0) && (strncmp(subdir, "/title", 6) != 0))
            continue;
        if (title_select && ((strncmp(subdir, "/title", 6) != 0) || (strchrcount(dir, '/') < 4)))
            continue;
        dirptr[n_dirs++] = dir;
        if (n_dirs * sizeof(char**) >= 0x8000)
            return 1;
    }
    if (n_dirs == 0) {
        Debug("No valid SD data found");
        return 1;
    } else { // sort the dir list
        bool swapped;
        do {
            swapped = false;
            for (u32 i = 0; i < n_dirs - 1; i++) {
                if (strncmp(dirptr[i], dirptr[i+1], 256) > 0) {
                    char* swap = dirptr[i];
                    dirptr[i] = dirptr[i+1];
                    dirptr[i+1] = swap;
                    swapped = true;
                }
            }
        } while (swapped);
    }
    
    // let the user choose a directory
    u32 index = 0;
    strncpy(path, dirptr[0], 128);
    DebugColor(COLOR_ASK, "Use arrow keys and <A> to choose a folder");
    while (true) {
        DebugColor(COLOR_SELECT, "\r%s", path + 13 + 33 + 33);
        u32 pad_state = InputWait();
        u32 cur_lvl = strchrcount(path, '/');
        if (pad_state & BUTTON_DOWN) { // find next path of same level
            do {
                if (++index >= n_dirs)
                    index = 0;
            } while (strchrcount(dirptr[index], '/') != cur_lvl);
        } else if (pad_state & BUTTON_UP) { // find prev path of same level
            do {
                index = (index) ? index - 1 : n_dirs - 1;
            } while (strchrcount(dirptr[index], '/') != cur_lvl);
        } else if ((pad_state & BUTTON_RIGHT) && (cur_lvl < 6)) { // up one level
            if ((index < n_dirs - 1) && (strchrcount(dirptr[index+1], '/') > cur_lvl))
                index++; // this only works because of the sorting of the dir list
        } else if ((pad_state & BUTTON_LEFT) && (cur_lvl > 4)) { // down one level
            while ((index > 0) && (cur_lvl <= strchrcount(dirptr[index], '/')))
                index--;
        } else if (pad_state & BUTTON_A) {
            DebugColor(COLOR_ASK, "%s", path + 13 + 33 + 33);
            break;
        } else if (pad_state & BUTTON_B) {
            DebugColor(COLOR_ASK, "(cancelled by user)");
            return 2;
        }
        strncpy(path, dirptr[index], 128);
    }
    
    return 0;
}

u32 CryptSdToSd(const char* filename, u32 offset, u32 size, CryptBufferInfo* info, bool handle_offset16)
{
    u8* buffer = BUFFER_ADDRESS;
    u32 offset_16 = (handle_offset16) ? offset % 16 : 0;
    u32 result = 0;

    // no DebugFileOpen() - at this point the file has already been checked enough
    if (!FileOpen(filename)) 
        return 1;

    info->buffer = buffer;
    if (offset_16) { // handle offset alignment / this assumes the data is >= 16 byte
        if(!DebugFileRead(buffer + offset_16, 16 - offset_16, offset)) {
            result = 1;
        }
        info->size = 16;
        CryptBuffer(info);
        if(!DebugFileWrite(buffer + offset_16, 16 - offset_16, offset)) {
            result = 1;
        }
    }
    for (u32 i = (offset_16) ? (16 - offset_16) : 0; i < size; i += BUFFER_MAX_SIZE) {
        u32 read_bytes = min(BUFFER_MAX_SIZE, (size - i));
        ShowProgress(i, size);
        if(!DebugFileRead(buffer, read_bytes, offset + i)) {
            result = 1;
            break;
        }
        info->size = read_bytes;
        CryptBuffer(info);
        if(!DebugFileWrite(buffer, read_bytes, offset + i)) {
            result = 1;
            break;
        }
    }

    ShowProgress(0, 0);
    FileClose();

    return result;
}

u32 VerifyNcch(const char* filename, u32 offset)
{
    NcchHeader* ncch = (NcchHeader*) 0x20316200;
    u8* exefs = (u8*) 0x20316400;
    char* status_str[3] = { "OK", "Fail", "-" }; 
    u32 ver_exthdr = 2;
    u32 ver_exefs = 2;
    u32 ver_romfs = 2;
    
    // some basic checks included - this only verifies decrypted NCCHs
    if (FileGetData(filename, (void*) ncch, 0x200, offset) != 0x200)
        return 1;
    if ((memcmp(ncch->magic, "NCCH", 4) != 0) || (!(ncch->flags[7] & 0x04)))
        return 1;

    // base hash checks for ExHeader / ExeFS / RomFS
    if (ncch->size_exthdr > 0)
        ver_exthdr = CheckHashFromFile(filename, offset + 0x200, 0x400, ncch->hash_exthdr);
    if (ncch->size_exefs_hash > 0)
        ver_exefs = CheckHashFromFile(filename, offset + (ncch->offset_exefs * 0x200), ncch->size_exefs_hash * 0x200, ncch->hash_exefs);
    if (ncch->size_romfs_hash > 0)
        ver_romfs = CheckHashFromFile(filename, offset + (ncch->offset_romfs * 0x200), ncch->size_romfs_hash * 0x200, ncch->hash_romfs);
    
    // thorough exefs verification
    if (ncch->size_exefs > 0) {
        u32 offset_byte = ncch->offset_exefs * 0x200;
        if (FileGetData(filename, exefs, 0x200, offset + offset_byte) != 0x200)
            ver_exefs = 1;
        for (u32 i = 0; (i < 10) && (ver_exefs != 1); i++) {
            u32 offset_exefs_file = offset_byte + getle32(exefs + (i*0x10) + 0x8) + 0x200;
            u32 size_exefs_file = getle32(exefs + (i*0x10) + 0xC);
            u8* hash_exefs_file = exefs + 0x200 - ((i+1)*0x20);
            if (size_exefs_file == 0)
                break;
            ver_exefs = CheckHashFromFile(filename, offset + offset_exefs_file, size_exefs_file, hash_exefs_file);
        }
    }
    
    // output results
    Debug("Verify ExHdr/ExeFS/RomFS: %s/%s/%s", status_str[ver_exthdr], status_str[ver_exefs], status_str[ver_romfs]);
    
    return (((ver_exthdr | ver_exefs | ver_romfs) & 1) == 0) ? 0 : 1;
}

u32 CryptNcch(const char* filename, u32 offset, u32 size, u64 seedId, u8* encrypt_flags)
{
    NcchHeader* ncch = (NcchHeader*) 0x20316200;
    u8* buffer = (u8*) 0x20316400;
    CryptBufferInfo info0 = {.setKeyY = 1, .keyslot = 0x2C, .mode = AES_CNT_CTRNAND_MODE};
    CryptBufferInfo info1 = {.setKeyY = 1, .mode = AES_CNT_CTRNAND_MODE};
    u8 seedKeyY[16] = { 0x00 };
    u32 result = 0;
    
    if (FileGetData(filename, (void*) ncch, 0x200, offset) != 0x200)
        return 1; // it's impossible to fail here anyways
 
    // check (again) for magic number
    if (memcmp(ncch->magic, "NCCH", 4) != 0) {
        Debug("Not a NCCH container");
        return 2; // not an actual error
    }
    
    // size plausibility check
    u32 size_sum = 0x200 + ((ncch->size_exthdr) ? 0x800 : 0x0) + 0x200 *
        (ncch->size_plain + ncch->size_logo + ncch->size_exefs + ncch->size_romfs);
    if (ncch->size * 0x200 < size_sum) {
        Debug("Probably not a NCCH container");
        return 2; // not an actual error
    }        
    
    // check if encrypted
    if (!encrypt_flags && (ncch->flags[7] & 0x04)) {
        Debug("NCCH is not encrypted");
        return 2; // not an actual error
    } else if (encrypt_flags && !(ncch->flags[7] & 0x04)) {
        Debug("NCCH is already encrypted");
        return 2; // not an actual error
    } else if (encrypt_flags && (encrypt_flags[7] & 0x04)) {
        Debug("Nothing to do!");
        return 2; // not an actual error
    }
    
    // check size
    if ((size > 0) && (ncch->size * 0x200 > size)) {
        Debug("NCCH size is out of bounds");
        return 1;
    }
    
    // select correct title ID for seed crypto
    if (seedId == 0) seedId = ncch->programId;
    
    // copy over encryption parameters (if applicable)
    if (encrypt_flags) {
        ncch->flags[3] = encrypt_flags[3];
        ncch->flags[7] &= (0x01|0x20|0x04)^0xFF;
        ncch->flags[7] |= (0x01|0x20)&encrypt_flags[7];
    }
    
    // check crypto type
    bool uses7xCrypto = ncch->flags[3];
    bool usesSeedCrypto = ncch->flags[7] & 0x20;
    bool usesSec3Crypto = (ncch->flags[3] == 0x0A);
    bool usesSec4Crypto = (ncch->flags[3] == 0x0B);
    bool usesFixedKey = ncch->flags[7] & 0x01;
    
    Debug("Code / Crypto: %.16s / %s%s%s%s", ncch->productcode, (usesFixedKey) ? "FixedKey " : "", (usesSec4Crypto) ? "Secure4 " : (usesSec3Crypto) ? "Secure3 " : (uses7xCrypto) ? "7x " : "", (usesSeedCrypto) ? "Seed " : "", (!uses7xCrypto && !usesSeedCrypto && !usesFixedKey) ? "Standard" : "");
    
    // setup zero key crypto
    if (usesFixedKey) {
        // from https://github.com/profi200/Project_CTR/blob/master/makerom/pki/dev.h
        u8 zeroKey[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        u8 sysKey[16]  = {0x52, 0x7C, 0xE6, 0x30, 0xA9, 0xCA, 0x30, 0x5F, 0x36, 0x96, 0xF3, 0xCD, 0xE9, 0x54, 0x19, 0x4B};
        uses7xCrypto = usesSeedCrypto = usesSec3Crypto = usesSec4Crypto = false;
        info1.setKeyY = info0.setKeyY = 0;
        info1.keyslot = info0.keyslot = 0x11;
        setup_aeskey(0x11, (ncch->programId & ((u64) 0x10 << 32)) ? sysKey : zeroKey);
    }
    
    // check 7x crypto
    if (uses7xCrypto && (CheckKeySlot(0x25, 'X') != 0)) {
        Debug("slot0x25KeyX not set up");
        Debug("This won't work on O3DS < 7.x or A9LH");
        return 1;
    }
    
    // check Secure3 crypto on O3DS
    if (usesSec3Crypto && (CheckKeySlot(0x18, 'X') != 0)) {
        Debug("slot0x18KeyX not set up");
        Debug("Secure3 crypto is not available");
        return 1;
    }
    
    // check Secure4 crypto
    if (usesSec4Crypto && (CheckKeySlot(0x1B, 'X') != 0)) {
        Debug("slot0x1BKeyX not set up");
        Debug("Secure4 crypto is not available");
        return 1;
    }
    
    // check / setup seed crypto
    if (usesSeedCrypto) {
        u8 seed[16];
        u32 found = 0;
        if (FindSeedInSeedSave(seed, seedId, ncch->hash_seed) == 0) { // try loading from NAND
            Debug("Loading seed from NAND: ok");
            found = 1;
        } else if (FileOpen("seeddb.bin")) { // try loading from seeddb.bin
            SeedInfoEntry* entry = (SeedInfoEntry*) buffer;
            for (u32 i = 0x10;; i += 0x20) {
                if (FileRead(entry, 0x20, i) != 0x20)
                    break;
                if (entry->titleId == seedId) {
                    memcpy(seed, entry->external_seed, 16);
                    Debug("Loading seed from seeddb.bin: ok");
                    found = 1;
                    break;
                }
            }
            FileClose();
        }
        if (found) {
            // validate seed
            if (ValidateSeed(seed, seedId, ncch->hash_seed) != 0) {
                Debug("Seed found, but validation failed!");
                Debug("Try fixing your seeddb.bin");
                return 1;
            }
            // set seed key
            u8 keydata[32];
            u8 sha256sum[32];
            memcpy(keydata, ncch->signature, 16);
            memcpy(keydata + 16, seed, 16);
            sha_quick(sha256sum, keydata, 32, SHA256_MODE);
            memcpy(seedKeyY, sha256sum, 16);
        } else {
            Debug("Seed not found in seeddb.bin or NAND!");
            Debug("Try updating your seeddb.bin");
            return 1;
        }
    }
    
    // basic setup of CryptBufferInfo structs
    memcpy(info0.keyY, ncch->signature, 16);
    memcpy(info1.keyY, (usesSeedCrypto) ? seedKeyY : ncch->signature, 16);
    info1.keyslot = (usesSec4Crypto) ? 0x1B : ((usesSec3Crypto) ? 0x18 : ((uses7xCrypto) ? 0x25 : info0.keyslot));
    
    Debug("%s ExHdr/ExeFS/RomFS (%ukB/%ukB/%uMB)",
        (encrypt_flags) ? "Encrypt" : "Decrypt",
        (ncch->size_exthdr > 0) ? 0x800 / 1024 : 0,
        (ncch->size_exefs * 0x200) / 1024,
        (ncch->size_romfs * 0x200) / (1024*1024));
        
    // process ExHeader
    if (ncch->size_exthdr > 0) {
        GetNcchCtr(info0.ctr, ncch, 1);
        result |= CryptSdToSd(filename, offset + 0x200, 0x800, &info0, true);
    }
    
    // process ExeFS
    if (ncch->size_exefs > 0) {
        u32 offset_byte = ncch->offset_exefs * 0x200;
        u32 size_byte = ncch->size_exefs * 0x200;
        if (uses7xCrypto || usesSeedCrypto) {
            GetNcchCtr(info0.ctr, ncch, 2);
            if (!encrypt_flags) // decrypt this first (when decrypting)
                result |= CryptSdToSd(filename, offset + offset_byte, 0x200, &info0, true);
            if (FileGetData(filename, buffer, 0x200, offset + offset_byte) != 0x200) // get exeFS header
                return 1;
            if (encrypt_flags) // encrypt this last (when encrypting)
                result |= CryptSdToSd(filename, offset + offset_byte, 0x200, &info0, true);
            // special ExeFS decryption routine ("banner" and "icon" use standard crypto)
            for (u32 i = 0; i < 10; i++) {
                char* name_exefs_file = (char*) buffer + (i*0x10);
                u32 offset_exefs_file = getle32(buffer + (i*0x10) + 0x8) + 0x200;
                u32 size_exefs_file = align(getle32(buffer + (i*0x10) + 0xC), 0x200);
                CryptBufferInfo* infoExeFs = ((strncmp(name_exefs_file, "banner", 8) == 0) ||
                    (strncmp(name_exefs_file, "icon", 8) == 0)) ? &info0 : &info1;
                if (size_exefs_file == 0)
                    continue;
                if (offset_exefs_file % 16) {
                    Debug("ExeFS file offset not aligned!");
                    result |= 1;
                    break; // this should not happen
                }
                GetNcchCtr(infoExeFs->ctr, ncch, 2);
                add_ctr(infoExeFs->ctr, offset_exefs_file / 0x10);
                infoExeFs->setKeyY = 1;
                result |= CryptSdToSd(filename, offset + offset_byte + offset_exefs_file,
                    align(size_exefs_file, 16), infoExeFs, true);
            }
        } else {
            GetNcchCtr(info0.ctr, ncch, 2);
            result |= CryptSdToSd(filename, offset + offset_byte, size_byte, &info0, true);
        }
    }
    
    // process RomFS
    if (ncch->size_romfs > 0) {
        GetNcchCtr(info1.ctr, ncch, 3);
        if (!usesFixedKey)
            info1.setKeyY = 1;
        result |= CryptSdToSd(filename, offset + (ncch->offset_romfs * 0x200), ncch->size_romfs * 0x200, &info1, true);
    }
    
    // set NCCH header flags
    if (!encrypt_flags) {
        ncch->flags[3] = 0x00;
        ncch->flags[7] &= (0x01|0x20)^0xFF;
        ncch->flags[7] |= 0x04;
    }
    
    // write header back
    if (!FileOpen(filename))
        return 1;
    if (!DebugFileWrite((void*) ncch, 0x200, offset)) {
        FileClose();
        return 1;
    }
    FileClose();
    
    
    return ((result == 0) && !encrypt_flags) ? VerifyNcch(filename, offset) : result;
}

u32 GetCiaInfo(CiaInfo* info, CiaHeader* header)
{
    info->offset_cert = align(header->size_header, 64);
    info->offset_ticket = info->offset_cert + align(header->size_cert, 64);
    info->offset_tmd = info->offset_ticket + align(header->size_ticket, 64);
    info->offset_content = info->offset_tmd + align(header->size_tmd, 64);
    info->offset_meta = (header->size_meta) ? info->offset_content + align(header->size_content, 64) : 0;
    info->offset_ticktmd = info->offset_ticket;
    info->offset_content_list = info->offset_tmd + sizeof(TitleMetaData);
    
    info->size_cert = header->size_cert;
    info->size_ticket = header->size_ticket;
    info->size_tmd = header->size_tmd;
    info->size_content = header->size_content;
    info->size_meta = header->size_meta;
    info->size_ticktmd = info->offset_content - info->offset_ticket;
    info->size_content_list = info->size_tmd - sizeof(TitleMetaData);
    info->size_cia = (header->size_meta) ? info->offset_meta + info->size_meta :
        info->offset_content + info->size_content;
    
    return 0;
}

u32 CryptCia(const char* filename, u8* ncch_crypt, bool cia_encrypt, bool cxi_only)
{
    const u8 sig_type[4] =  { 0x00, 0x01, 0x00, 0x04 };
    u8* buffer = (u8*) 0x20316600;
    __attribute__((aligned(16))) u8 titlekey[16];
    CiaInfo cia;
    u64 titleId;
    u32 content_count;
    u32 result = 0;
    
    if (cia_encrypt) // process only one layer when encrypting
        ncch_crypt = NULL;
    
    if (!FileOpen(filename)) // already checked this file
        return 1;
    if (!DebugFileRead(buffer, 0x20, 0x00)) {
        FileClose();
        return 1;
    }
    
    // get offsets for various sections & check
    if (GetCiaInfo(&cia, (CiaHeader*) buffer) != 0)
        return 1;
    
    if (FileGetSize() < cia.size_cia) {
        Debug("Not a CIA or corrupt file");
        FileClose();
        return 1;
    }
    
    if (cia.size_ticktmd > 0x10000) {
        Debug("Ticket/TMD too big");
        FileClose();
        return 1;
    }
    
    // load ticket & tmd to buffer, close file
    if (!DebugFileRead(buffer, cia.size_ticktmd, cia.offset_ticktmd)) {
        FileClose();
        return 1;
    }
    FileClose();
    
    Ticket* ticket = (Ticket*) buffer;
    TitleMetaData* tmd = (TitleMetaData*) (buffer + align(cia.size_ticket, 64));
    TmdContentChunk* content_list = (TmdContentChunk*) (tmd + 1);
    if (memcmp(ticket->sig_type, sig_type, 4) != 0) {
        Debug("Bad ticket signature type");
        return 1; // this should never happen
    }
    if (memcmp(tmd->sig_type, sig_type, 4) != 0) {
        Debug("Bad TMD signature type");
        return 1; // this should never happen
    }
    
    // extract & decrypt titlekey
    if (cia.size_ticket != 0x140 + 0x210) {
        Debug("Ticket is too small (%i byte)", cia.size_ticket);
        return 1;
    }
    TitleKeyEntry titlekeyEntry;
    titleId = getbe64(ticket->title_id);
    memcpy(titlekeyEntry.titleId, ticket->title_id, 8);
    memcpy(titlekeyEntry.titleKey, ticket->titlekey, 16);
    titlekeyEntry.commonKeyIndex = ticket->commonkey_idx;
    CryptTitlekey(&titlekeyEntry, false);
    memcpy(titlekey, titlekeyEntry.titleKey, 16);
    
    // get content data from TMD
    content_count = getbe16(tmd->content_count);
    if (content_count * 0x30 != cia.size_tmd - (0x140 + 0xC4 + (64 * 0x24))) {
        Debug("TMD content count (%i) / list size mismatch", content_count);
        return 1;
    }
    u64 size_tmd_content = 0;
    for (u32 i = 0; i < content_count; i++)
        size_tmd_content += getbe64(content_list[i].size);
    if (size_tmd_content != cia.size_content) {
        Debug("TMD content size / actual size mismatch");
        return 1;
    }
    
    bool untouched = true;
    u32 n_processed = 0;
    u32 next_offset = cia.offset_content;
    CryptBufferInfo info = {.setKeyY = 0, .keyslot = 0x11, .mode = (cia_encrypt) ? AES_CNT_TITLEKEY_ENCRYPT_MODE : AES_CNT_TITLEKEY_DECRYPT_MODE};
    setup_aeskey(0x11, titlekey);
    
    if (ncch_crypt)
        Debug("Pass #1: CIA decryption...");
    if (cxi_only) content_count = 1;
    for (u32 i = 0; i < content_count; i++) {
        u32 size = (u32) getbe64(content_list[i].size);
        u32 offset = next_offset;
        next_offset = offset + size;
        if (!(content_list[i].type[1] & 0x1) != cia_encrypt)
            continue; // depending on 'cia_encrypt' setting: not/already encrypted
        untouched = false;
        if (cia_encrypt) {
            Debug("Verifying unencrypted content...");
            if (CheckHashFromFile(filename, offset, size, content_list[i].hash) != 0) {
                Debug("Verification failed!");
                result = 1;
                continue;
            }
            Debug("Verified OK!");
        }
        Debug("%scrypting Content %i of %i (%iMB)...", (cia_encrypt) ? "En" : "De", i + 1, content_count, size / (1024*1024));
        memset(info.ctr, 0x00, 16);
        memcpy(info.ctr, content_list[i].index, 2);
        if (CryptSdToSd(filename, offset, size, &info, true) != 0) {
            Debug("%scryption failed!", (cia_encrypt) ? "En" : "De");
            result = 1;
            continue;
        }
        if (!cia_encrypt) {
            Debug("Verifying decrypted content...");
            if (CheckHashFromFile(filename, offset, size, content_list[i].hash) != 0) {
                Debug("Verification failed!");
                result = 1;
                continue;
            }
            Debug("Verified OK!");
        }
        content_list[i].type[1] ^= 0x1;
        n_processed++;
    }
    
    if (ncch_crypt && (result == 0)) {
        Debug("Pass #2: NCCH decryption...");
        next_offset = cia.offset_content;
        for (u32 i = 0; i < content_count; i++) {
            u32 ncch_state;
            u32 size = getbe32(content_list + (0x30 * i) + 0xC);
            u32 offset = next_offset;
            next_offset = offset + size;
            if (content_list[i].type[1] & 0x1)
                continue; // skip this if still CIA (shallow) encrypted
            Debug("Processing Content %i of %i (%iMB)...", i + 1, content_count, size / (1024*1024));
            ncch_state = CryptNcch(filename, offset, size, titleId, NULL);
            if (!(ncch_crypt[7] & 0x04) && (ncch_state != 1))
                ncch_state = CryptNcch(filename, offset, size, titleId, ncch_crypt);
            if (ncch_state == 0) {
                untouched = false;
                Debug("Recalculating hash...");
                if (GetHashFromFile(filename, offset, size, content_list[i].hash) != 0) {
                    Debug("Recalculation failed!");
                    result = 1;
                    continue;
                }
            } else if (ncch_state == 1) {
                Debug("Failed decrypting NCCH!");
                result = 1;
                continue;
            }
            n_processed++;
        }
    }
    
    if (untouched) {
        Debug((cia_encrypt) ? "CIA is already encrypted" : "CIA is not encrypted");
    } else if (n_processed > 0) {
        // recalculate content info hashes
        Debug("Recalculating TMD hashes...");
        for (u32 i = 0, kc = 0; i < 64 && kc < content_count; i++) {
            TmdContentInfo* cntinfo = tmd->contentinfo + i;
            u32 k = getbe16(cntinfo->cmd_count);
            sha_quick(cntinfo->hash, content_list + kc, k * sizeof(TmdContentChunk), SHA256_MODE);
            kc += k;
        }
        sha_quick(tmd->contentinfo_hash, (u8*)tmd->contentinfo, 64 * sizeof(TmdContentInfo), SHA256_MODE);
        // write ticket / tmd
        if (!FileOpen(filename)) // already checked this file
            return 1;
        if (!DebugFileWrite(buffer, cia.size_ticktmd, cia.offset_ticktmd))
            result = 1;
        FileClose();
    }
    
    return result;
}

u32 CryptBoss(const char* filename, bool encrypt)
{
    const u8 boss_magic[] = {0x62, 0x6F, 0x73, 0x73, 0x00, 0x01, 0x00, 0x01};
    CryptBufferInfo info = {.setKeyY = 0, .keyslot = 0x38, .mode = AES_CNT_CTRNAND_MODE};
    u8 boss_header[0x28];
    u8 content_header[0x14] = { 0x00 };
    u8 content_header_sha256[0x20];
    u8 l_sha256[0x20];
    u32 fsize = 0;
    bool encrypted = true;
    
    // base BOSS file checks
    if (!FileOpen(filename)) // already checked this file
        return 1;
    if (!DebugFileRead(boss_header, 0x28, 0x00)) {
        FileClose();
        return 1;
    }
    if (memcmp(boss_magic, boss_header, 8) != 0) {
        Debug("Not a BOSS file");
        FileClose();
        return 1;
    }
    fsize = getbe32(boss_header + 8);
    if ((fsize != FileGetSize()) || (fsize < 0x52)) {
        Debug("BOSS file has bad size");
        FileClose();
        return 1;
    }
    FileClose();

    // BOSS file verification / encryption check
    if (FileGetData(filename, content_header, 0x12, 0x28) != 0x12)
        return 1;
    if (FileGetData(filename, content_header_sha256, 0x20, 0x3A) != 0x20)
        return 1;
    sha_quick(l_sha256, content_header, 0x14, SHA256_MODE);
    encrypted = (memcmp(content_header_sha256, l_sha256, 0x20) != 0);
    if (encrypt == encrypted) {
        Debug("BOSS is already %scrypted", (encrypted) ? "en" : "de");
        return 1;
    }
    
    if (!encrypted) {
        Debug("BOSS verification: OK");
    }
    
    // BOSS file decryption
    Debug("%scrypting BOSS...", (encrypt) ? "En" : "De");
    memset(info.ctr, 0x00, 16);
    memcpy(info.ctr, boss_header + 0x1C, 12);
    info.ctr[15] = 0x01;
    Debug("CTR: %08X%08X%08X%08X", getbe32(info.ctr), getbe32(info.ctr + 4), getbe32(info.ctr + 8), getbe32(info.ctr + 12));
    CryptSdToSd(filename, 0x28, fsize - 0x28, &info, false);
    
    // BOSS file verification (#2)
    if (!encrypt) {
        if (FileGetData(filename, content_header, 0x12, 0x28) != 0x12)
            return 1;
        if (FileGetData(filename, content_header_sha256, 0x20, 0x3A) != 0x20)
            return 1;
        sha_quick(l_sha256, content_header, 0x14, SHA256_MODE);
        if (memcmp(content_header_sha256, l_sha256, 0x20) == 0) {
            Debug("BOSS verification: OK");
        } else {
            Debug("BOSS verification: Failed");
            return 1;
        }
    }
    
    return 0;
}

u32 CryptGameFiles(u32 param)
{
    u8 ncch_crypt_none[8]     = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04 };
    u8 ncch_crypt_standard[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    u8 ncch_crypt_zerokey[8]  = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 };
    const u8 boss_magic[] = {0x62, 0x6F, 0x73, 0x73, 0x00, 0x01, 0x00, 0x01};
    const char* ncsd_partition_name[8] = {
        "Executable", "Manual", "DPC", "Unknown", "Unknown", "Unknown", "UpdateN3DS", "UpdateO3DS" 
    };
    const char* batch_dir = GetGameDir();
    u8* buffer = (u8*) 0x20316000;
    
    bool batch_ncch = param & GC_NCCH_PROCESS;
    bool batch_cia = param & GC_CIA_PROCESS;
    bool batch_boss = param & GC_BOSS_PROCESS;
    bool boss_encrypt = param & GC_BOSS_ENCRYPT;
    bool cia_encrypt = param & GC_CIA_ENCRYPT;
    bool cxi_only = param & GC_CXI_ONLY;
    u8* ncch_crypt = (param & GC_NCCH_ENC0x2C) ? ncch_crypt_standard :
        (param & GC_NCCH_ENCZERO) ? ncch_crypt_zerokey : NULL;
    u8* cia_ncch_crypt = (param & GC_CIA_DEEP) ? ncch_crypt_none : ncch_crypt;
    
    u32 n_processed = 0;
    u32 n_failed = 0;
    
    if (!batch_dir || !DebugDirOpen(batch_dir)) {
        Debug("Game directory not found!");
        Debug("(check readme for more info)");
        return 1;
    }
    Debug("");
    
    char path[256];
    u32 path_len = strnlen(batch_dir, 128);
    memcpy(path, batch_dir, path_len);
    path[path_len++] = '/';
    
    while (DirRead(path + path_len, 256 - path_len)) {
        if (FileGetData(path, buffer, 0x200, 0x0) != 0x200)
            continue;
        
        if (batch_ncch && (memcmp(buffer + 0x100, "NCCH", 4) == 0)) {
            Debug("Processing NCCH \"%s\"", path + path_len);
            if (CryptNcch(path, 0x00, 0, 0, ncch_crypt) != 1) {
                Debug("Success!");
                n_processed++;
            } else {
                Debug("Failed!");
                n_failed++;
            }
            Debug("");
        } else if (batch_ncch && (memcmp(buffer + 0x100, "NCSD", 4) == 0)) {
            if (getle64(buffer + 0x110) != 0) 
                continue; // skip NAND backup NCSDs
            Debug("Processing NCSD \"%s\"", path + path_len);
            NcsdHeader* ncsd = (NcsdHeader*) buffer;
            u32 p;
            u32 nc = (cxi_only) ? 1 : 8;
            for (p = 0; p < nc; p++) {
                u64 seedId = (p) ? ncsd->mediaId : 0;
                u32 offset = ncsd->partitions[p].offset * 0x200;
                u32 size = ncsd->partitions[p].size * 0x200;
                if (size == 0) 
                    continue;
                Debug("Partition %i (%s)", p, ncsd_partition_name[p]);
                if (CryptNcch(path, offset, size, seedId, ncch_crypt) == 1)
                    break;
            }
            if ( p == nc ) {
                Debug("Success!");
                n_processed++;
            } else {
                Debug("Failed!");
                n_failed++;
            }
            Debug("");
        } else if (batch_cia && (memcmp(buffer, "\x20\x20", 2) == 0)) {
            Debug("Processing CIA \"%s\"", path + path_len);
            if (CryptCia(path, cia_ncch_crypt, cia_encrypt, cxi_only) == 0) {
                Debug("Success!");
                n_processed++;
            } else {
                Debug("Failed!");
                n_failed++;
            }
            Debug("");
        } else if (batch_boss && (memcmp(buffer, boss_magic, 8) == 0)) {
            Debug("Processing BOSS \"%s\"", path + path_len);
            if (CryptBoss(path, boss_encrypt) == 0) {
                Debug("Success!");
                n_processed++;
            } else {
                Debug("Failed!");
                n_failed++;
            }
            Debug("");
        }
    }
    
    DirClose();
    
    if (n_processed) {
        Debug("%ux processed / %ux failed ", n_processed, n_failed);
    } else if (!n_failed) {
        Debug("Nothing found in %s/!", batch_dir);
    }
    
    return !n_processed;
}

u32 BuildCiaStubTmd(u8* stub, TitleMetaData* tmd, u32 size_tmd)
{
    // stub should have at least room for 16KiB (0x4000)
    const u8 sig_type[4] =  { 0x00, 0x01, 0x00, 0x04 };
    CiaInfo cia;
    
    
    // check TMD sig type and size
    u32 content_count = getbe16(tmd->content_count);
    if ((memcmp(tmd->sig_type, sig_type, 4) != 0) ||
        (size_tmd != sizeof(TitleMetaData) + (content_count * sizeof(TmdContentChunk)))) {
        Debug("Provided TMD is corrupt");
        return 0;
    }
    
    // set everything zero for a clean start
    memset(stub, 0, 0x4000);
    
    // CIA header
    CiaHeader* header = (CiaHeader*) stub;
    header->size_header = sizeof(CiaHeader);
    header->size_cert = CIA_CERT_SIZE;
    header->size_ticket = sizeof(Ticket);
    header->size_tmd = size_tmd;
    header->size_meta = sizeof(CiaMeta);
    
    // Extract info from TMD content list
    TmdContentChunk* content_list = (TmdContentChunk*) (tmd + 1);
    u8* title_id = tmd->title_id;
    for (u32 i = 0; i < content_count; i++) {
        u32 index = getbe16(content_list[i].index);
        header->size_content += getbe64(content_list[i].size);
        header->content_index[index/8] |= (1 << (7-(index%8)));
    }
    
    // Get CIA info
    GetCiaInfo(&cia, header);
    
    // Certificate chain
    // Thanks go to ihaveamac for discovering this and the offsets
    const u8 cert_hash_expected[0x20] = {
        0xC7, 0x2E, 0x1C, 0xA5, 0x61, 0xDC, 0x9B, 0xC8, 0x05, 0x58, 0x58, 0x9C, 0x63, 0x08, 0x1C, 0x8A,
        0x10, 0x78, 0xDF, 0x42, 0x99, 0x80, 0x3A, 0x68, 0x58, 0xF0, 0x41, 0xF9, 0xCB, 0x10, 0xE6, 0x35
    };
    u8* cert = (u8*) (stub + cia.offset_cert);
    u8* cert_db = (u8*) BUFFER_ADDRESS; // should be okay to use this area
    PartitionInfo* p_ctrnand = GetPartitionInfo(P_CTRNAND);
    u32 offset_db, size_db;
    if ((SeekFileInNand(&offset_db, &size_db, "DBS        CERTS   DB ", p_ctrnand) != 0) || (size_db != 0x6000)){
        Debug("certs.db not found or bad size");
        return 0;
    }
    if (DecryptNandToMem(cert_db, offset_db, size_db, p_ctrnand) != 0)
        return 0;
    memcpy(cert + 0x000, cert_db + 0x0C10, 0x1F0);
    memcpy(cert + 0x1F0, cert_db + 0x3A00, 0x210);
    memcpy(cert + 0x400, cert_db + 0x3F10, 0x300);
    memcpy(cert + 0x700, cert_db + 0x3C10, 0x300);
    u8 cert_hash[0x20];
    sha_quick(cert_hash, cert, CIA_CERT_SIZE, SHA256_MODE);
    if (memcmp(cert_hash, cert_hash_expected, 0x20) != 0) {
        Debug("Error generating certificate chain");
        return 0;
    }
    
    // Ticket
    u8 ticket_cnt_index[] = { // whatever this is
        0x00, 0x01, 0x00, 0x14, 0x00, 0x00, 0x00, 0xAC, 0x00, 0x00, 0x00, 0x14, 0x00, 0x01, 0x00, 0x14,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x84,
        0x00, 0x00, 0x00, 0x84, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
    };
    Ticket* ticket = (Ticket*) (stub + cia.offset_ticket);
    memcpy(ticket->sig_type, sig_type, 4);
    memset(ticket->signature, 0xFF, 0x100);
    snprintf((char*) ticket->issuer, 0x40, "Root-CA00000003-XS0000000c");
    memset(ticket->ecdsa, 0xFF, 0x3C);
    ticket->version = 0x01;
    memset(ticket->titlekey, 0xFF, 16);
    memcpy(ticket->title_id, title_id, 8);
    ticket->commonkey_idx = 0x01;
    ticket->audit = 0x01; // whatever
    memcpy(ticket->content_index, ticket_cnt_index, sizeof(ticket_cnt_index));
    
    // TMD
    memcpy(stub + cia.offset_tmd, tmd, size_tmd);
    
    
    return cia.offset_content;
}

u32 BuildCiaStubNcch(u8* stub, u8* ncchncsd)
{
    const u8 sig_type[4] =  { 0x00, 0x01, 0x00, 0x04 };
    TitleMetaData* tmd = (TitleMetaData*) 0x20320000;
    TmdContentChunk* content_list = (TmdContentChunk*) (tmd + 1);
    u64 content_size[3] = { 0 };
    u8 content_type[3] = { 0x00 };
    u32 content_count = 0;
    
    
    // set TMD all zero for a clean start
    memset(tmd, 0x00, sizeof(TitleMetaData) + (3 * sizeof(TmdContentChunk)));
    
    // check type of provided ncchncsd header, extract data for TMD
    if (memcmp(ncchncsd + 0x100, "NCCH", 4) == 0) {
        NcchHeader* ncch = (NcchHeader*) ncchncsd;
        content_count = 1;
        content_size[0] = ncch->size * 0x200;
        content_type[0] = 0;
        for (u32 i = 0; i < 8; i++)
            tmd->title_id[i] = ((u8*) &(ncch->partitionId))[7-i];
    } else if (memcmp(ncchncsd + 0x100, "NCSD", 4) == 0) {
        NcsdHeader* ncsd = (NcsdHeader*) ncchncsd;
        for (u32 p = 0; p < 3; p++) {
            if (ncsd->partitions[p].size) {
                content_size[content_count] = ncsd->partitions[p].size * 0x200;
                content_type[content_count++] = p;
            }
        }
        for (u32 i = 0; i < 8; i++)
            tmd->title_id[i] = ((u8*) &(ncsd->mediaId))[7-i];
    } else {
        Debug("Bad NCCH/NCSD header"); // meaning: developer did not pay attention
        return 0;
    }
    
    // TMD
    memcpy(tmd->sig_type, sig_type, 4);
    memset(tmd->signature, 0xFF, 0x100);
    snprintf((char*) tmd->issuer, 0x40, "Root-CA00000003-CP0000000b");
    tmd->version = 0x01;
    tmd->title_type[3] = 0x40; // whatever
    memset(tmd->save_size, 0x00, 4); // placeholder
    tmd->content_count[1] = (u8) content_count;
    memset(tmd->contentinfo_hash, 0xFF, 0x20); // placeholder (hash)
    tmd->contentinfo[0].cmd_count[1] = (u8) content_count;
    memset(tmd->contentinfo[0].hash, 0xFF, 0x20); // placeholder (hash)
    
    // TMD content list
    for (u32 i = 0; i < content_count; i++) {
        content_list[i].id[3] = content_list[i].index[1] = content_type[i];
        for (u32 j = 0; j < 8; j++) // content size
            content_list[i].size[j] = (u8) (content_size[i] >> (8*(7-j)));
        memset(content_list[i].hash, 0xFF, 0x20); // placeholder (content hash)
    }
    
    
    return BuildCiaStubTmd(stub, tmd, sizeof(TitleMetaData) + (content_count * sizeof(TmdContentChunk)));
}

u32 FinalizeCiaFile(const char* filename, bool meta_only)
{
    u8* buffer = (u8*) 0x20316000;
    NcchHeader* ncch = (NcchHeader*) (0x20316000 + 0x4000);
    u8* exthdr = (u8*) (0x20316000 + 0x4200); // we only need the first 0x400 byte
    CiaMeta* meta = (CiaMeta*) BUFFER_ADDRESS;
    CiaInfo cia;
    
    
    // fetch CIA info, Ticket, TMD, content_list
    if ((FileGetData(filename, buffer, 0x4000, 0) != 0x4000) || (memcmp(buffer, "\x20\x20", 2) != 0)) {
        Debug("This does not look like a CIA file"); // checking an arbitrary size here
        return 1;
    }
    GetCiaInfo(&cia, (CiaHeader*) buffer);
    if (cia.offset_content > 0x4000) {
        Debug("CIA stub has bad size (%lu)", cia.offset_content);
        return 1;
    }
    TitleMetaData* tmd = (TitleMetaData*) (buffer + cia.offset_tmd);
    TmdContentChunk* content_list = (TmdContentChunk*) (tmd + 1);
    
    // For first content: fix NCCH ExHeader, build metadata
    u32 content_count = getbe16(tmd->content_count);
    if (content_count && (getbe16(content_list[0].index) == 0)) {
        if ((FileGetData(filename, ncch, 0x600, cia.offset_content) != 0x600) || (memcmp(ncch->magic, "NCCH", 4) != 0)) {
            Debug("Failed reading NCCH content");
            return 1;
        }
        
        // init metadata with all zeroes
        memset(meta, 0x00, sizeof(CiaMeta));
        meta->core_version = 2;
        
        // prepare crypto stuff (even if it may not get used)
        CryptBufferInfo info = {.setKeyY = 1, .keyslot = 0x2C, .buffer = exthdr, .size = 0x400, .mode = AES_CNT_CTRNAND_MODE};
        memcpy(info.keyY, ncch->signature, 16);
        if (ncch->flags[7] & 0x01) { // set up zerokey crypto instead
            __attribute__((aligned(16))) u8 zeroKey[16] =
                {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            info.setKeyY = 0;
            info.keyslot = 0x11;
            setup_aeskey(0x11, zeroKey);
            use_aeskey(0x11);
        }
        
        // process extheader
        if (ncch->size_exthdr > 0) {
            if (!(ncch->flags[7] & 0x04)) { // encrypted NCCH
                GetNcchCtr(info.ctr, ncch, 1);
                CryptBuffer(&info);
            }
            if (!meta_only) {
                exthdr[0xD] |= (1<<1); // set SD flag
                memcpy(tmd->save_size, exthdr + 0x1C0, 4); // get save size for CXI
                sha_quick(ncch->hash_exthdr, exthdr, 0x400, SHA256_MODE); // fix exheader hash
            }
            memcpy(meta->dependencies, exthdr + 0x40, 0x180); // copy dependencies to meta
            if (!(ncch->flags[7] & 0x04)) { // encrypted NCCH
                GetNcchCtr(info.ctr, ncch, 1);
                CryptBuffer(&info);
            }
        }
        
        // process ExeFS (for SMDH)
        if (ncch->size_exefs > 0) {
            u8 exefs_hdr[0x200];
            u32 offset_exefs = ncch->offset_exefs * 0x200;
            if (FileGetData(filename, exefs_hdr, 0x200, cia.offset_content + offset_exefs) != 0x200) {
                Debug("Failed reading NCCH ExeFS content");
                return 1;
            }
            if (!(ncch->flags[7] & 0x04)) { // encrypted ExeFS
                info.buffer = exefs_hdr;
                info.size = 0x200;
                GetNcchCtr(info.ctr, ncch, 2);
                CryptBuffer(&info);
            }
            for (u32 j = 0; j < 10; j++) { // search for icon
                char* name_exefs_file = (char*) exefs_hdr + (j*0x10);
                u32 offset_exefs_file = getle32(exefs_hdr + (j*0x10) + 0x8) + 0x200;
                u32 size_exefs_file = align(getle32(exefs_hdr + (j*0x10) + 0xC), 0x10);
                if ((size_exefs_file > 0) && (size_exefs_file <= 0x36C0) && !(offset_exefs_file % 16) &&
                    (strncmp(name_exefs_file, "icon", 8) == 0)) {
                    if (FileGetData(filename, meta->smdh, size_exefs_file,
                        cia.offset_content + offset_exefs + offset_exefs_file) != size_exefs_file) {
                        Debug("Failed reading NCCH ExeFS SMDH");
                        return 1;
                    }
                    if (!(ncch->flags[7] & 0x04)) { // encrypted ExeFS SMDH
                        info.buffer = meta->smdh;
                        info.size = size_exefs_file;
                        GetNcchCtr(info.ctr, ncch, 2);
                        add_ctr(info.ctr, offset_exefs_file / 0x10);
                        CryptBuffer(&info);
                    }
                    break;
                }
            }
        }
        
        // inject NCCH / exthdr back & append metadata
        if (!FileOpen(filename))
            return 1;
        if ((!meta_only && !DebugFileWrite(ncch, 0x400, cia.offset_content)) ||
            !DebugFileWrite(meta, sizeof(CiaMeta), cia.offset_meta)) {
            FileClose();
            return 1;
        }
        FileClose();
    }
    
    if (!meta_only) {
        // fix content hashes
        u32 next_offset = cia.offset_content;
        for (u32 i = 0; i < content_count; i++) {
            u32 size = (u32) getbe64(content_list[i].size);
            u32 offset = next_offset;
            next_offset = offset + size;
            // (re)calculate hash
            if (GetHashFromFile(filename, offset, size, content_list[i].hash) != 0) {
                Debug("Hash recalculation failed!");
                return 1;
            }
        }
        
        // fix other TMD hashes
        for (u32 i = 0, kc = 0; i < 64 && kc < content_count; i++) {
            TmdContentInfo* cntinfo = tmd->contentinfo + i;
            u32 k = getbe16(cntinfo->cmd_count);
            sha_quick(cntinfo->hash, content_list + kc, k * sizeof(TmdContentChunk), SHA256_MODE);
            kc += k;
        }
        sha_quick(tmd->contentinfo_hash, (u8*)tmd->contentinfo, 64 * sizeof(TmdContentInfo), SHA256_MODE);
        
        // inject fixed TMD back to CIA file
        if (!FileOpen(filename))
            return 1;
        if (!DebugFileWrite(tmd, cia.size_tmd, cia.offset_tmd)) {
            FileClose();
            return 1;
        }
        FileClose();
    }
    
    
    return 0;
}

u32 ConvertNcsdNcchToCia(u32 param)
{
    (void) (param); // param is unused here
    u8* buffer = BUFFER_ADDRESS;
    u8* stub = (u8*) 0x20316000;
    
    u32 n_processed = 0;
    u32 n_failed = 0;
    
    const char* batch_dir = GetGameDir();
    if (!batch_dir || !DebugDirOpen(batch_dir)) {
        Debug("Game directory not found!");
        Debug("(check readme for more info)");
        return 1;
    }
    
    char path[256];
    u32 path_len = strnlen(batch_dir, 128);
    memcpy(path, batch_dir, path_len);
    path[path_len++] = '/';
    
    while (DirRead(path + path_len, 256 - path_len)) {
        u8 header[0x200];
        if (FileGetData(path, header, 0x200, 0x0) != 0x200)
            continue;
        if ((memcmp(header + 0x100, "NCCH", 4) != 0) && (memcmp(header + 0x100, "NCSD", 4) != 0))
            continue;
        if ((memcmp(header + 0x100, "NCSD", 4) == 0) && (getle64(header + 0x110) != 0)) 
            continue; // skip NAND backup NCSDs
        
        n_failed++; // this will be set back upon completion
        Debug("");
        Debug("Converting to CIA: %s", path + path_len);
        
        // build the CIA stub
        u32 stub_size = BuildCiaStubNcch(stub, header);
        if (stub_size == 0) {
            Debug("Failed building the CIA stub");
            continue;
        }
        
        // write it (first round)
        char filename[256];
        Debug("Writing CIA stub (%lu byte)...", stub_size);
        snprintf(filename, 256, "/%s.cia", path);
        if (FileDumpData(filename, stub, stub_size) != stub_size) {
            Debug("Failed writing the CIA stub");
            continue;
        }
        
        // insert content file(s)
        if (!FileOpen(path))
            continue; // this will not happen here
        if (memcmp(header + 0x100, "NCCH", 4) == 0) { // for NCCH files
            NcchHeader* ncch = (NcchHeader*) header;
            u32 size = ncch->size * 0x200;
            u32 offset = stub_size;
            Debug("Injecting NCCH content (%luMB)...", size / (1024 * 1024));
            if (FileInjectTo(filename, 0, offset, size, false, buffer, BUFFER_MAX_SIZE) != size) {
                Debug("Content not readable or has bad size");
                FileClose();
                continue;
            }
        } else { // can only be a NCSD file at this point
            NcsdHeader* ncsd = (NcsdHeader*) header;
            u32 next_offset = stub_size;
            u32 p;
            for (p = 0; p < 3; p++) {
                u32 size = ncsd->partitions[p].size * 0x200;
                u32 offset_ncch = ncsd->partitions[p].offset * 0x200;
                u32 offset = next_offset;
                if (!size)
                    continue;
                next_offset += size;
                Debug("Injecting NCSD content %lu (%luMB)...", p, size / (1024 * 1024));
                if (FileInjectTo(filename, offset_ncch, offset, size, false, buffer, BUFFER_MAX_SIZE) != size) {
                    Debug("Content not readable or has bad size");
                    break;
                }
            }
            if (p < 3) {
                FileClose();
                continue;
            }
        }
        FileClose();
        
        // Fix the CIA file
        Debug("Finalizing CIA file...");
        if (FinalizeCiaFile(filename, false) != 0) {
            Debug("Failed!");
            continue;
        }
        
        Debug("Success!");
        n_failed--;
        n_processed++;
    }
    
    if (n_processed) {
        Debug("");
        Debug("%ux processed / %ux failed ", n_processed, n_failed);
    } else if (!n_failed) {
        Debug("Nothing found in %s/!", batch_dir);
    }
    
    return !n_processed;
}

u32 CryptSdFiles(u32 param)
{
    (void) (param); // param is unused here
    const char* subpaths[] = {"dbs", "extdata", "title", NULL};
    const char* batch_dir = GetGameDir();
    u32 n_processed = 0;
    u32 n_failed = 0;
    u32 plen = 0;
    
    if (!batch_dir) {
        Debug("Game directory not found!");
        Debug("(check readme for more info)");
        return 1;
    }
    plen = strnlen(batch_dir, 128);
    
    // setup AES key from SD
    SetupMovableKeyY(false, 0x34, NULL);
    
    // main processing loop
    for (u32 s = 0; subpaths[s] != NULL; s++) {
        char* filelist = (char*) 0x20400000;
        char basepath[128];
        u32 bplen;
        Debug("Processing subpath \"%s\"...", subpaths[s]);
        sprintf(basepath, "%s/%s", batch_dir, subpaths[s]);
        if (!GetFileList(basepath, filelist, 0x100000, true, true, false)) {
            Debug("Not found!");
            continue;
        }
        bplen = strnlen(basepath, 128);
        for (char* path = strtok(filelist, "\n"); path != NULL; path = strtok(NULL, "\n")) {
            u32 fsize = 0;
            CryptBufferInfo info = {.keyslot = 0x34, .setKeyY = 0, .mode = AES_CNT_CTRNAND_MODE};
            GetSdCtr(info.ctr, path + plen);
            if (FileOpen(path)) {
                fsize = FileGetSize();
                FileClose();
            } else {
                Debug("Could not open: %s", path + bplen);
                n_failed++;
                continue;
            }
            Debug("%2u: %s", n_processed, path + bplen);
            if (CryptSdToSd(path, 0, fsize, &info, true) == 0) {
                n_processed++;
            } else {
                Debug("Failed!");
                n_failed++;
                break;
            }
        }
    }
    
    return (n_processed) ? 0 : 1;
}

u32 DecryptSdFilesDirect(u32 param)
{
    (void) (param); // param is unused here
    char* filelist = (char*) 0x20400000;
    u8 movable_keyY[16] = { 0 };
    char basepath[256];
    const char* batch_dir = GetGameDir();
    u32 n_processed = 0;
    u32 n_failed = 0;
    u32 bplen = 0;
    
    if (!batch_dir) {
        Debug("Game directory not found!");
        Debug("(check readme for more info)");
        return 1;
    }
    
    if (SetupMovableKeyY(true, 0x34, movable_keyY) != 0)
        return 1; // movable.sed has to be present in NAND
    
    Debug("");
    if (SdFolderSelector(basepath, movable_keyY, false) != 0)
        return 1;
    if (!GetFileList(basepath, filelist, 0x100000, true, true, false)) {
        Debug("Nothing found in folder");
        return 1;
    }
    Debug("");
    
    Debug("Using base path %s", basepath);
    bplen = strnlen(basepath, 256);
    
    // main processing loop
    for (char* srcpath = strtok(filelist, "\n"); srcpath != NULL; srcpath = strtok(NULL, "\n")) {
        char* subpath = srcpath + 13 + 33 + 33; // length of ("/Nintendo 3DS" + "/<id0>" + "/<id1>")
        char dstpath[256];
        u32 fsize = 0;
        snprintf(dstpath, 256, "%s%s", batch_dir, subpath);
        CryptBufferInfo info = {.keyslot = 0x34, .setKeyY = 0, .mode = AES_CNT_CTRNAND_MODE};
        GetSdCtr(info.ctr, subpath);
        Debug("%2u: %s", n_processed, srcpath + bplen);
        if (FileOpen(srcpath)) {
            fsize = FileGetSize();
            if (!DebugCheckFreeSpace(fsize))
                return 1;
            if (FileCopyTo(dstpath, BUFFER_ADDRESS, BUFFER_MAX_SIZE) != fsize) {
                Debug("Could not copy: %s", srcpath + bplen);
                n_failed++;
                break;
            }
            FileClose();
        } else {
            Debug("Could not open: %s", srcpath + bplen);
            n_failed++;
            continue;
        }
        if (CryptSdToSd(dstpath, 0, fsize, &info, true) == 0) {
            n_processed++;
        } else {
            Debug("Failed!");
            n_failed++;
            break;
        }
    }
    
    return (n_processed) ? 0 : 1;
}

u32 ConvertSdToCia(u32 param)
{
    char* filelist = (char*) 0x20400000;
    u8 movable_keyY[16] = { 0 };
    char titlepath[256];
    const char* dest_dir = GetGameDir();
    
    u32 n_processed = 0;
    u32 n_failed = 0;
    
    if (!dest_dir) {
        Debug("Game directory not found!");
        Debug("(check readme for more info)");
        return 1;
    }
    
    if (SetupMovableKeyY(true, 0x34, movable_keyY) != 0)
        return 1; // movable.sed has to be present in NAND
    
    Debug("");
    if (SdFolderSelector(titlepath, movable_keyY, true) != 0)
        return 1;
    if (!GetFileList(titlepath, filelist, 0x100000, true, true, false)) {
        Debug("Nothing found in folder");
        return 1;
    }
    Debug("");
    
    
    // main processing loop
    for (char* srcpath = strtok(filelist, "\n"); srcpath != NULL; srcpath = strtok(NULL, "\n")) {
        u8* buffer = BUFFER_ADDRESS;
        u8* stub = (u8*) 0x20316000;
        TitleMetaData* tmd = (TitleMetaData*) 0x20320000;
        TmdContentChunk* content_list = (TmdContentChunk*) (tmd + 1);
        CryptBufferInfo info = {.keyslot = 0x34, .setKeyY = 0, .mode = AES_CNT_CTRNAND_MODE};
        char ciapath[128];
        
        snprintf(titlepath, 256, srcpath);
        char* subpath = titlepath + 13 + 33 + 33; // length of ("/Nintendo 3DS" + "/<id0>" + "/<id1>")
        char* filename = subpath + 6 + 9 + 9 + 8; // position of the actual filename
        
        // get title ID
        u32 tid_low, tid_high, tmd_num;
        if ((sscanf(subpath, "/title/%08lX/%08lX/content/%08lX.tmd", &tid_high, &tid_low, &tmd_num) != 3) ||
            (strncmp(filename + 9, ".tmd", 16) != 0))
            continue;
        snprintf(ciapath, 128, "/%s/%08lX%08lX.cia", dest_dir, tid_high, tid_low);
        Debug("Building CIA from ID %08lX%08lX", tid_high, tid_low);
        
        // this is set back after success
        n_failed++;
        
        // TMD file
        u32 tmd_size;
        Debug("Fetching TMD...");
        if (FileOpen(titlepath)) {
            tmd_size = FileGetSize();
            if (!DebugFileRead(tmd, (tmd_size > 0x4000) ? 0x4000 : tmd_size, 0)) {
                FileClose();
                continue;
            }
            FileClose();
        }
        info.buffer = (u8*) tmd;
        info.size = tmd_size;
        GetSdCtr(info.ctr, subpath);
        CryptBuffer(&info);
        
        // CIA stub
        u32 stub_size = BuildCiaStubTmd(stub, tmd, tmd_size);
        if (stub_size == 0) {
            Debug("Failed building the CIA stub");
            continue;
        }
        CiaInfo cia;
        GetCiaInfo(&cia, (CiaHeader*) stub);
        tmd = (TitleMetaData*) (stub + cia.offset_tmd);
        content_list = (TmdContentChunk*) (tmd + 1);
        
        // search for proper ticket
        u32 t_offset, t_size;
        bool tik_legit = false;
        PartitionInfo* ctrnand_info = GetPartitionInfo(P_CTRNAND);
        Ticket* ticket = (Ticket*) (stub + cia.offset_ticket);
        Debug("Searching for proper ticket...");
        if (SeekFileInNand(&t_offset, &t_size, "DBS        TICKET  DB ", ctrnand_info) == 0) {
            for (u32 offset = 0; (offset < t_size) && !tik_legit; offset += BUFFER_MAX_SIZE - (2 * NAND_SECTOR_SIZE)) {
                const u8 sig_type[4] =  { 0x00, 0x01, 0x00, 0x04 };
                u32 read_bytes = min(BUFFER_MAX_SIZE, (t_size - offset));
                ShowProgress(offset, t_size);
                if (DecryptNandToMem(buffer, t_offset + offset, read_bytes, ctrnand_info) != 0)
                    continue;
                for (u32 i = 0x140; (i < read_bytes - 0x210) && !tik_legit; i++) {
                    if (((memcmp(buffer + i, (u8*) "Root-CA00000003-XS0000000c", 26) == 0) ||
                        (memcmp(buffer + i, (u8*) "Root-CA00000004-XS00000009", 26) == 0)) &&
                        (memcmp(buffer + i - 0x140, sig_type, 4) == 0)) {
                        // u32 consoleId = getle32(buffer + i + 0x98);
                        u8* titleId = buffer + i + 0x9C;
                        if (memcmp(titleId, ticket->title_id, 8) == 0) {
                            Debug("Found ticket, injecting...");
                            memcpy(ticket, buffer + i - 0x140, sizeof(Ticket));
                            tik_legit = true;
                        }
                    }
                }
                if (DebugCheckCancel())
                    return 1;
            }
        }
        
        if (!tik_legit)
            Debug("Ticket not found, skipped");
        
        // wipe console unique info
        if (getbe32(ticket->console_id) || getbe32(ticket->eshop_id)) {
            Debug("Console unique info found, wiping...");
            memset(ticket->console_id, 0, 4); // zero out console id
            memset(ticket->eshop_id, 0, 4); // zero out eshop id
            memset(ticket->ticket_id, 0, 8); // zero out ticket id
        }
        
        // not (no more?) a legit ticket
        if (!getbe64(ticket->ticket_id))
            tik_legit = false;
        
        // not a legit ticket or decrypted param, no need to reencrypt
        if (!tik_legit || (param & GC_CIA_DEEP)) {
            u32 content_count = getbe16(tmd->content_count);
            if (!tik_legit && !(param & GC_CIA_DEEP))
                Debug("Ticket not legit, disabling reencrypt...");
            for (u32 c = 0; c < content_count; c++)
                content_list[c].type[1] &= (0xFF^0x1);
            for (u32 i = 0, kc = 0; i < 64 && kc < content_count; i++) {
                TmdContentInfo* cntinfo = tmd->contentinfo + i;
                u32 k = getbe16(cntinfo->cmd_count);
                sha_quick(cntinfo->hash, content_list + kc, k * sizeof(TmdContentChunk), SHA256_MODE);
                kc += k;
            }
            sha_quick(tmd->contentinfo_hash, (u8*)tmd->contentinfo, 64 * sizeof(TmdContentInfo), SHA256_MODE);
        }  else { // setup slot 0x11 for titlekey crypto
            TitleKeyEntry titlekeyEntry;
            memcpy(titlekeyEntry.titleId, ticket->title_id, 8);
            memcpy(titlekeyEntry.titleKey, ticket->titlekey, 16);
            titlekeyEntry.commonKeyIndex = ticket->commonkey_idx;
            CryptTitlekey(&titlekeyEntry, false);
            setup_aeskey(0x11, titlekeyEntry.titleKey);
        }
        
        // write CIA stub
        Debug("Writing CIA stub (%lu byte)...", stub_size);
        if (FileDumpData(ciapath, stub, stub_size) != stub_size) {
            Debug("Failed writing the CIA stub");
            break;
        }
        
        // inject content file(s)
        u32 c = 0;
        u32 next_offset = stub_size;
        u32 content_count = getbe16(tmd->content_count);
        for (c = 0; c < content_count; c++) {
            u32 id = getbe32(content_list[c].id);
            u32 size = (u32) getbe64(content_list[c].size);
            u32 offset = next_offset;
            next_offset = offset + size;
            snprintf(filename, 16, "/%08lx.app", id);
            Debug("Injecting content id %08lX (%lu kB)...", id, size / 1024);
            if (!FileOpen(titlepath)) {
                Debug("Content not found");
                break;
            }
            if (FileInjectTo(ciapath, 0, offset, size, false, buffer, BUFFER_MAX_SIZE) != size) {
                Debug("Content has bad size");
                FileClose();
                break;
            }
            FileClose();
            Debug("Decrypting content id %08lX (SD)...", id);
            GetSdCtr(info.ctr, subpath);
            if (CryptSdToSd(ciapath, offset, size, &info, false) != 0) {
                Debug("Failed decrypting content");
                break;
            }
            if (param & GC_CIA_DEEP) {
                Debug("Decrypting content id %08lX (NCCH)...", id);
                if (CryptNcch(ciapath, offset, size, 0, NULL) == 1) {
                    Debug("Failed decrypting content");
                    break;
                }
            }
        }
        if (c < content_count)
            break; // error, skip the remaing steps
        
        // finalize the CIA file
        Debug("Finalizing CIA file...");
        if (FinalizeCiaFile(ciapath, !(param & GC_CIA_DEEP)) != 0) {
            Debug("Failed!");
            break;
        }
        
        next_offset = stub_size;
        for (c = 0; c < content_count; c++) {
            if (content_list[c].type[1] & 0x1) { // redo titlekey crypto (because hashes & signatures)
                u32 id = getbe32(content_list[c].id);
                u32 size = (u32) getbe64(content_list[c].size);
                u32 offset = next_offset;
                next_offset = offset + size;
                CryptBufferInfo info_tk = {.setKeyY = 0, .keyslot = 0x11, .mode = AES_CNT_TITLEKEY_ENCRYPT_MODE};
                memset(info_tk.ctr, 0x00, 16);
                memcpy(info_tk.ctr, content_list[c].index, 2);
                Debug("Reencrypting content id %08lX...", id);
                if (CryptSdToSd(ciapath, offset, size, &info_tk, false) != 0) {
                    Debug("Failed encrypting content");
                    break;
                }
            }
        }
        if (c < content_count)
            break; // error, leave loop
        
        n_failed--;
        n_processed++;
        Debug("");
    }
    
    if (n_processed || n_failed) {
        Debug("%ux generated / %ux failed", n_processed, n_failed);
    } else {
        Debug("No usable content found");
    }
    
    
    return !n_processed;
}

u32 DecryptSdToCxi(u32 param)
{
    (void) (param); // param is unused here
    char* filelist = (char*) 0x20400000;
    u8 movable_keyY[16] = { 0 };
    char titlepath[256];
    const char* dest_dir = GetGameDir();
    
    u32 n_processed = 0;
    u32 n_failed = 0;
    
    if (!dest_dir) {
        Debug("Game directory not found!");
        Debug("(check readme for more info)");
        return 1;
    }
    
    if (SetupMovableKeyY(true, 0x34, movable_keyY) != 0)
        return 1; // movable.sed has to be present in NAND
    
    Debug("");
    if (SdFolderSelector(titlepath, movable_keyY, true) != 0)
        return 1;
    if (!GetFileList(titlepath, filelist, 0x100000, true, true, false)) {
        Debug("Nothing found in folder");
        return 1;
    }
    Debug("");
    
    
    // main processing loop
    for (char* srcpath = strtok(filelist, "\n"); srcpath != NULL; srcpath = strtok(NULL, "\n")) {
        u8* buffer = BUFFER_ADDRESS;
        TitleMetaData* tmd = (TitleMetaData*) 0x20320000;
        TmdContentChunk* content_list = (TmdContentChunk*) (tmd + 1);
        CryptBufferInfo info = {.keyslot = 0x34, .setKeyY = 0, .mode = AES_CNT_CTRNAND_MODE};
        char cxipath[128];
        
        snprintf(titlepath, 256, srcpath);
        char* subpath = titlepath + 13 + 33 + 33; // length of ("/Nintendo 3DS" + "/<id0>" + "/<id1>")
        char* filename = subpath + 6 + 9 + 9 + 8; // position of the actual filename
        
        // get title ID
        u32 tid_low, tid_high, tmd_num;
        if ((sscanf(subpath, "/title/%08lX/%08lX/content/%08lX.tmd", &tid_high, &tid_low, &tmd_num) != 3) ||
            (strncmp(filename + 9, ".tmd", 16) != 0))
            continue;
        snprintf(cxipath, 128, "/%s/%08lX%08lX.cxi", dest_dir, tid_high, tid_low);
        Debug("Decrypting CXI from ID %08lX%08lX", tid_high, tid_low);
        
        // this is set back after success
        n_failed++;
        
        // TMD file
        u32 tmd_size;
        if (FileOpen(titlepath)) {
            tmd_size = FileGetSize();
            if (!DebugFileRead(tmd, (tmd_size > 0x4000) ? 0x4000 : tmd_size, 0)) {
                FileClose();
                continue;
            }
            FileClose();
        }
        info.buffer = (u8*) tmd;
        info.size = tmd_size;
        GetSdCtr(info.ctr, subpath);
        CryptBuffer(&info);
        
        // check for empty content list
        if (!getbe16(tmd->content_count)) {
            Debug("Content list is empty"); // won't happen
            continue;
        }
        
        // copy first content
        u32 id = getbe32(content_list[0].id);
        u32 size = (u32) getbe64(content_list[0].size);
        snprintf(filename, 16, "/%08lx.app", id);
        Debug("Copying content id %08lX (%lu kB)...", id, size / 1024);
        if (!FileOpen(titlepath)) {
            Debug("Content not found");
            continue;
        }
        if (FileCopyTo(cxipath, buffer, BUFFER_MAX_SIZE) != size) {
            Debug("Content has bad size");
            FileClose();
            continue;
        }
        FileClose();
        
        // SD decryption
        Debug("Decrypting (SD) content id %08lX...", id);
        GetSdCtr(info.ctr, subpath);
        if (CryptSdToSd(cxipath, 0, size, &info, false) != 0) {
            Debug("Failed decrypting content");
            continue;
        }
        
        // NCCH decryption
        Debug("Decrypting (NCCH) content id %08lX...", id);
        if (CryptNcch(cxipath, 0, size, 0, NULL) == 1)
            continue;
        
        n_failed--;
        n_processed++;
        Debug("");
    }
    
    if (n_processed || n_failed) {
        Debug("%ux generated / %ux failed", n_processed, n_failed);
    } else {
        Debug("No usable content found");
    }
    
    
    return !n_processed;
}

static u32 DumpCartToFile(u32 offset_cart, u32 offset_file, u32 size, u32 total, CryptBufferInfo* info, u8* out)
{
    // this assumes cart dumping initialized & file open for writing
    // also, careful, uses standard buffer
    u8* buffer = BUFFER_ADDRESS;
    u32 result = 0;
    
    if (info) {
        info->buffer = buffer;
    }
    
    // if offset_cart does not start at sector boundary
    if (offset_cart % 0x200) { 
        u32 read_bytes = 0x200 - (offset_cart % 0x200);
        Cart_Dummy();
        Cart_Dummy();
        CTR_CmdReadData(offset_cart / 0x200, 0x200, 1, buffer);
        memmove(buffer, buffer + (offset_cart % 0x200), read_bytes);
        if (info) {
            info->size = read_bytes;
            CryptBuffer(info);
        }
        if (out) {
            memcpy(out, buffer, read_bytes);
            out += read_bytes;
        }
        if (!DebugFileWrite(buffer, read_bytes, offset_file)) 
           return 1;
        offset_cart += read_bytes;
        offset_file += read_bytes;
    }
    
    for (u64 i = 0; i < size; i += CART_CHUNK_SIZE) {
        u32 read_bytes = min(CART_CHUNK_SIZE, (size - i));
        if (total)
            ShowProgress(offset_file + i, total);
        Cart_Dummy();
        Cart_Dummy();
        CTR_CmdReadData((offset_cart + i) / 0x200, 0x200, (read_bytes + 0x1FF) / 0x200, buffer);
        if (info) {
            info->size = read_bytes;
            CryptBuffer(info);
        }
        if (out) {
            memcpy(out, buffer, read_bytes);
            out += read_bytes;
        }
        if (!DebugFileWrite(buffer, read_bytes, offset_file + i)) {
            result = 1;
            break;
        }
    }
    ShowProgress(0, 0);
    
    return result;
}

static u32 DecryptCartNcchToFile(u32 offset_cart, u32 offset_file, u32 size, u32 total)
{
    // this assumes cart dumping to be initialized already and file open for writing(!)
    // algorithm is simplified / slimmed when compared to CryptNcch()
    // and only has required capabilities
    NcchHeader* ncch = (NcchHeader*) 0x20317000;
    u8* exefs = (u8*) 0x20317200;
    CryptBufferInfo info = {.setKeyY = 0, .mode = AES_CNT_CTRNAND_MODE};
    u32 slot_base = 0x2C;
    u32 slot_7x = 0x2C;
    
    // read header
    Cart_Dummy();
    Cart_Dummy();
    CTR_CmdReadData(offset_cart / 0x200, 0x200, 1, ncch);
    
    // check header, set up stuff
    if ((memcmp(ncch->magic, "NCCH", 4) != 0) || (ncch->size > (size / 0x200))) {
        Debug("Error reading partition NCCH header");
        return 1;
    }
    
    // check crypto, setup crypto
    if (ncch->flags[7] & 0x04) { // for unencrypted partitions...
        Debug("Not encrypted, dumping instead...");
        return DumpCartToFile(offset_cart, offset_file, size, total, NULL, NULL);
    } else if (ncch->flags[7] & 0x1) { // zeroKey / fixedKey crypto
        // from https://github.com/profi200/Project_CTR/blob/master/makerom/pki/dev.h
        u8 zeroKey[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        u8 sysKey[16]  = {0x52, 0x7C, 0xE6, 0x30, 0xA9, 0xCA, 0x30, 0x5F, 0x36, 0x96, 0xF3, 0xCD, 0xE9, 0x54, 0x19, 0x4B};
        slot_base = slot_7x = 0x11;
        setup_aeskey(0x11, (ncch->programId & ((u64) 0x10 << 32)) ? sysKey : zeroKey);
        use_aeskey(0x11);
    } else if (ncch->flags[3]) { // 7x crypto type
        slot_7x = (ncch->flags[3] == 0x0A) ? 0x18 : (ncch->flags[3] == 0x0B) ? 0x1B : 0x25;
        if (CheckKeySlot(slot_7x, 'X') != 0) {
            Debug("Slot0x%02XKeyX not set up", slot_7x);
            return 1;
        }
        setup_aeskeyY(slot_7x, ncch->signature);
        use_aeskey(slot_7x);
        setup_aeskeyY(slot_base, ncch->signature);
        use_aeskey(slot_base);
    } else { // standard crypto type (pre 7x)
        setup_aeskeyY(slot_base, ncch->signature);
        use_aeskey(slot_base);
    }
    
    // disable crypto in header, write to file
    ncch->flags[3] = 0x00;
    ncch->flags[7] &= (0x01|0x20)^0xFF;
    ncch->flags[7] |= 0x04;
    if (!DebugFileWrite(ncch, 0x200, offset_file))
        return 1;
        
    // process ExHeader
    if (ncch->size_exthdr > 0) {
        GetNcchCtr(info.ctr, ncch, 1);
        info.keyslot = slot_base;
        if (DumpCartToFile(offset_cart + 0x200, offset_file + 0x200, 0x800, total, &info, NULL) != 0)
            return 1;
    }
    
    // logo region / plain region
    if (ncch->offset_exefs > 5) {
        if (DumpCartToFile(offset_cart + 0xA00, offset_file + 0xA00, (ncch->offset_exefs - 5) * 0x200, total, NULL, NULL) != 0)
            return 1;
    }
    
    // process ExeFS
    if (ncch->size_exefs > 0) {
        u32 offset_exefs = ncch->offset_exefs * 0x200;
        // include space between ExeFS and RomFS if possible
        u32 size_exefs = ((ncch->offset_romfs) ? (ncch->offset_romfs - ncch->offset_exefs) :
            ncch->size_exefs) * 0x200;
        // dump the whole thing encrypted, then overwrite with decrypted
        if (DumpCartToFile(offset_cart + offset_exefs, offset_file + offset_exefs, size_exefs, total, NULL, NULL) != 0)
            return 1;
        // using 7x crypto routines for everything
        GetNcchCtr(info.ctr, ncch, 2);
        info.keyslot = slot_base;
        if (DumpCartToFile(offset_cart + offset_exefs, offset_file + offset_exefs, 0x200, total, &info, exefs) != 0)
            return 1;
        for (u32 i = 0; i < 10; i++) {
            char* name = (char*) exefs + (i*0x10);
            u32 offset_exefs_file = getle32(exefs + (i*0x10) + 0x8) + 0x200;
            u32 size_exefs_file = align(getle32(exefs + (i*0x10) + 0xC), 0x200);
            if (!size_exefs_file)
                continue;
            GetNcchCtr(info.ctr, ncch, 2);
            add_ctr(info.ctr, offset_exefs_file / 0x10);
            info.keyslot = ((strncmp(name, "banner", 8) == 0) || (strncmp(name, "icon", 8) == 0)) ? slot_base : slot_7x;
            if (DumpCartToFile(offset_cart + offset_exefs + offset_exefs_file,
                offset_file + offset_exefs + offset_exefs_file, size_exefs_file, total, &info, NULL) != 0)
                return 1;
        }
    }
    
    // process RomFS
    if (ncch->size_romfs > 0) {
        GetNcchCtr(info.ctr, ncch, 3);
        info.keyslot = slot_7x;
        if (DumpCartToFile(offset_cart + (ncch->offset_romfs * 0x200),
            offset_file + (ncch->offset_romfs * 0x200), (ncch->size_romfs * 0x200), total, &info, NULL) != 0)
            return 1;
    }
    
    return 0;
}

u32 DumpCtrGameCart(u32 param)
{
    NcsdHeader* ncsd = (NcsdHeader*) 0x20316000;
    NcchHeader* ncch = (NcchHeader*) 0x20317000;
    CiaHeader* cia_stub = (CiaHeader*) 0x2031A000;
    CiaInfo cia;
    char filename[64];
    u64 cart_size = 0;
    u64 data_size = 0;
    u64 dump_size = 0;
    u64 card2_offset = 0;
    u32 result = 0;

    // read cartridge NCCH header
    CTR_CmdReadHeader(ncch);
    if (memcmp(ncch->magic, "NCCH", 4) != 0) {
        Debug("Error reading cart NCCH header");
        return 1;
    }

    // secure init
    u32 sec_keys[4];
    Cart_Secure_Init((u32*) ncch, sec_keys);
    
    // read NCSD header
    Cart_Dummy();
    CTR_CmdReadData(0, 0x200, 0x1000 / 0x200, ncsd);
    if (memcmp(ncsd->magic, "NCSD", 4) != 0) {
        Debug("Error reading cart NCSD header");
        return 1;
    }
    
    // check for card2 area offset
    // see: https://www.3dbrew.org/wiki/NCSD#Partition_Flags
    // see: https://www.3dbrew.org/wiki/NCSD#Card_Info_Header
    if ((ncsd->partition_flags[5] == 0x2) && (getle32(((u8*) ncsd) + 0x200) != 0xFFFFFFFF))
        card2_offset = (u64) getle32(((u8*) ncsd) + 0x200) * 0x200;
    
    // check NCSD partition table
    cart_size = (u64) ncsd->size * 0x200;
    for (u32 i = 0; i < 8; i++) {
        NcchPartition* partition = ncsd->partitions + i;
        if ((partition->offset == 0) && (partition->size == 0))
            continue;
        if (partition->offset < (data_size / 0x200)) {
            Debug("Overlapping partitions in NCSD table");
            return 1; // should never happen
        }
        data_size = (u64) (partition->offset + partition->size) * 0x200;
    }
    
    // output some info
    Debug("Product ID: %.16s", ncch->productcode);
    Debug("Product version: %02u", ((u8*)ncsd)[0x312]);
    Debug("Cartridge data size: %lluMB", cart_size / 0x100000);
    Debug("Cartridge used size: %lluMB", data_size / 0x100000);
    if (data_size > cart_size) {
        Debug("Used size exceeds cartridge size");
        return 1; // should never happen
    } else if ((data_size < 0x4000) || (cart_size < 0x4000)) {
        Debug("Bad cartridge size");
        return 1; // should never happen
    }
    if (param & CD_MAKECIA) {
        if (BuildCiaStubNcch((u8*) cia_stub, (u8*) ncsd) == 0)
            return 1;
        GetCiaInfo(&cia, cia_stub);
        dump_size = cia.size_cia;
        Debug("Cartridge CIA size : %lluMB", dump_size / 0x100000);
        if (dump_size >= 0x100000000) { // should not happen
            Debug("Error: Too big for the FAT32 file system");
            return 1;
        }
    } else {
        dump_size = (param & CD_TRIM) ? data_size : cart_size;
        Debug("Cartridge dump size: %lluMB", dump_size / 0x100000);
        if ((dump_size == 0x100000000) && (data_size < dump_size)) {
            dump_size -= 0x200; // silently remove the last sector for 4GB ROMs
        } else if (dump_size >= 0x100000000) { // should not happen
            Debug("Error: Too big for the FAT32 file system");
            if (!(param & CD_TRIM))
                Debug("(maybe try dumping trimmed?)");
            return 1;
        }
    }
    
    if ((param & CD_TRIM) && card2_offset)
        Debug("Warning: Trimming CARD2 game removes save area");
    
    if (!DebugCheckFreeSpace((size_t) dump_size))
        return 1;
    
    // create file, write CIA / NCSD header
    Debug("");
    snprintf(filename, 64, "/%s%s%.16s_%02u%s.%s",
        GetGameDir() ? GetGameDir() : "",
        GetGameDir() ? "/" : "",
        ncch->productcode,
        ((u8*)ncsd)[0x312],	// version
        (param & CD_DECRYPT) ? "-dec" : "",
        (param & CD_MAKECIA) ? "cia" : "3ds");
    if (!FileCreate(filename, true)) {
        Debug("Could not create output file on SD");
        return 1;
    }
    if (param & CD_DECRYPT) { // fix the flags inside the NCCH copy for decrypted
        ncch->flags[3] = 0x00;
        ncch->flags[7] &= (0x01|0x20)^0xFF;
        ncch->flags[7] |= 0x04;
    }
    if (param & CD_MAKECIA) { // CIA stub, including header, cert, ticket, TMD
        if (!DebugFileWrite((void*) cia_stub, cia.offset_content, 0)) {
            FileClose();
            return 1;
        }
    } else { // NCSD: first 0x4000 byte, including NCSD header
        memset(((u8*) ncsd) + 0x1200, 0xFF, 0x4000 - 0x1200);
        if (!DebugFileWrite((void*) ncsd, 0x4000, 0)) {
            FileClose();
            return 1;
        }
    }
    
    if (param & CD_MAKECIA) {
        u32 next_offset = cia.offset_content;
        u32 p;
        for (p = 0; p < 3; p++) {
            u32 size = ncsd->partitions[p].size * 0x200;
            u32 offset_cart = ncsd->partitions[p].offset * 0x200;
            u32 offset_file = next_offset;
            if (size == 0) 
                continue;
            next_offset += size;
            if (param & CD_DECRYPT) {
                Debug("Decrypting partition #%lu (%luMB)...", p, size / 0x100000);
                if (DecryptCartNcchToFile(offset_cart, offset_file, size, dump_size) != 0)
                    break;
            } else {
                Debug("Dumping partition #%lu (%luMB)...", p, size / 0x100000);
                if (DumpCartToFile(offset_cart, offset_file, size, dump_size, NULL, NULL) != 0)
                    break;
            } 
        }
        if (param & CD_DECRYPT)
            Debug("Decryption %s!", (p == 3) ? "success!" : "failed!");
        if (p != 3)
            result = 1;
    } else if (!(param & CD_DECRYPT)) { // dump the encrypted cart
        Debug("Dumping cartridge %.16s (%lluMB)...", ncch->productcode, dump_size / 0x100000);
        result = DumpCartToFile(0x4000, 0x4000, dump_size - 0x4000, dump_size, NULL, NULL);
    } else { // dump decrypted partitions
        u32 p;
        for (p = 0; p < 8; p++) {
            u32 offset = ncsd->partitions[p].offset * 0x200;
            u32 size = ncsd->partitions[p].size * 0x200;
            if (size == 0) 
                continue;
            Debug("Decrypting partition #%lu (%luMB)...", p, size / 0x100000);
            if (DecryptCartNcchToFile(offset, offset, size, dump_size) != 0)
                break;
        }
        if (p == 8) {
            Debug("Decryption success!");
        } else {
            Debug("Decryption failed!");
            result = 1;
        }
        
        if ((result == 0) && (dump_size > data_size)) {
            Debug("Dumping padding (%lluMB)...", (dump_size - data_size) / 0x100000);
            result = DumpCartToFile(data_size, data_size, dump_size - data_size, dump_size, NULL, NULL);
        }
    }
    FileClose();
    
    if (result == 0) { // finalizing steps
        if (param & CD_MAKECIA) {
            Debug("Finalizing CIA file...");
            if (FinalizeCiaFile(filename, false) != 0)
                result = 1;
        } else if ((card2_offset >= data_size) && (card2_offset < dump_size)) {
            u8* buffer = BUFFER_ADDRESS;
            memset(buffer, 0xFF, BUFFER_MAX_SIZE);
            Debug("Wiping CARD2 area (%lluMB)...", (dump_size - card2_offset) / 0x100000);
            if (FileOpen(filename)) {
                for (u32 i = card2_offset; i < dump_size; i += BUFFER_MAX_SIZE) {
                    if (!DebugFileWrite(buffer, min(BUFFER_MAX_SIZE, (dump_size - i)), i)) {
                        result = 1;
                        break;
                    }
                }
                FileClose();
            } else {
                result = 1;
            }
        }
    }
    
    // verify decrypted ROM
    if ((result == 0) && (param & CD_DECRYPT)) {
        Debug("");
        if (param & CD_MAKECIA) {
            u32 next_offset = cia.offset_content;
            for (u32 p = 0; p < 3; p++) {
                u32 offset = next_offset;
                u32 size = ncsd->partitions[p].size * 0x200;
                if (size == 0) 
                    continue;
                next_offset += size;
                Debug("Verifiying partition #%lu (%luMB)...", p, size / 0x100000);
                if (VerifyNcch(filename, offset) != 0)
                    result = 1;
            }
        } else {
            for (u32 p = 0; p < 8; p++) {
                u32 offset = ncsd->partitions[p].offset * 0x200;
                u32 size = ncsd->partitions[p].size * 0x200;
                if (size == 0) 
                    continue;
                Debug("Verifiying partition #%lu (%luMB)...", p, size / 0x100000);
                if (VerifyNcch(filename, offset) != 0)
                    result = 1;
            }
        }
        Debug("Verification %s", (result == 0) ? "success!" : "failed!");
    }
    
    
    return result;
}

u32 DumpTwlGameCart(u32 param)
{
    char filename[64];
    u64 cart_size = 0;
    u64 data_size = 0;
    u64 dump_size = 0;
    u8* dsibuff = BUFFER_ADDRESS;
    u8* buff = BUFFER_ADDRESS+0x8000;
    u64 offset = 0x8000;
    u32 arm9iromOffset = -1;
    int isDSi = 0;

    memset (buff, 0x00, 0x8000);


    NTR_CmdReadHeader (buff);
    if (buff[0] == 0x00) {
        Debug("Error reading cart header");
        return 1;
    }

    // Unitcode (00h=NDS, 02h=NDS+DSi, 03h=DSi) (bit1=DSi)
    isDSi = (buff[0x12] != 0x00);
    Debug("Product name: %.12s", (char*) &buff[0x00]);
    Debug("Product ID: %.6s", (char*) &buff[0x0C]);
    Debug("Product version: %02u", buff[0x1E]);

    cart_size = (128 * 1024) << buff[0x14];
    if (isDSi) {
        // NTR used data size field does not include TWL-specific data.
        // Use the TWL field.
        data_size = *((u32*)&buff[0x210]);
    } else {
        data_size = *((u32*)&buff[0x80]);
    }
    dump_size = (param & CD_TRIM) ? data_size : cart_size;
    Debug("Cartridge data size: %lluMB", cart_size / 0x100000);
    Debug("Cartridge used size: %lluMB", data_size / 0x100000);
    Debug("Cartridge dump size: %lluMB", dump_size / 0x100000);
    
    if (data_size > cart_size) {
        Debug("Used size exceeds cartridge size");
        return 1; // should never happen
    }

    if (!NTR_Secure_Init (buff, Cart_GetID(), 0)) {
        Debug("Error reading secure data");
        return 1;
    }

    Debug("");
    snprintf(filename, 64, "/%s%s%.6s_%02u.nds",
        GetGameDir() ? GetGameDir() : "",
        GetGameDir() ? "/" : "",
        (const char*)&buff[0x0C], buff[0x1E]);

    if (!DebugFileCreate(filename, true))
        return 1;
    if (!DebugFileWrite(buff, 0x8000, 0)) {
        FileClose();
        return 1;
    }
    
    if (isDSi) {
        // initialize cartridge
        Cart_Init();
        //Cart_GetID();
        
        NTR_CmdReadHeader (dsibuff);
        
        if (!NTR_Secure_Init (dsibuff, Cart_GetID(), 1)) {
            Debug("Error reading dsi secure data");
            //return 1;
        }
        
        arm9iromOffset = *((u32*)&dsibuff[0x1C0]);
    }

    u32 stop = 0;
    for (offset=0x8000;offset < dump_size;offset+=CART_CHUNK_SIZE) {
        if( (offset + CART_CHUNK_SIZE) > dump_size)
            stop = (offset + CART_CHUNK_SIZE)-dump_size; // correct over-sized writes with "stop" variable
        for(u32 i=0; i < CART_CHUNK_SIZE; i += 0x200) {
            NTR_CmdReadData (offset+i, buff+i);
        }
        if (!DebugFileWrite((void*) buff, CART_CHUNK_SIZE - stop, offset)) {
            FileClose();
            return 1;
        }
        ShowProgress(offset, dump_size);
    }
    
    if (isDSi && !DebugFileWrite(dsibuff+0x4000, 0x4000, arm9iromOffset)) {
        FileClose();
        return 1;
    }
    
    FileClose ();
    ShowProgress(0, 0);
    return 0;
}

u32 DumpGameCart(u32 param)
{
    u32 cartId;
    
    // check if cartridge inserted
    if (REG_CARDCONF2 & 0x1) {
        Debug("Cartridge was not detected");
        return 1;
    }
    
    // initialize cartridge
    Cart_Init();
    cartId = Cart_GetID();
    Debug("Cartridge ID: %08X", Cart_GetID());
    Debug("Cartridge Type: %s", (cartId & 0x10000000) ? "CTR" : "NTR/TWL");
    
    // check options vs. cartridge type
    if (!(cartId & 0x10000000) && (param & CD_MAKECIA)) {
        Debug("NTR/TWL carts can't be dumped to CIA");
        return 1;
    }
    if (!(cartId & 0x10000000) && (param & CD_DECRYPT)) {
        Debug("NTR/TWL carts are not encrypted, won't decrypt");
    }

    return (cartId & 0x10000000) ? DumpCtrGameCart(param) : DumpTwlGameCart(param);
}

u32 DumpPrivateHeader(u32 param)
{
    (void) param;
    NcchHeader* ncch = (NcchHeader*) 0x20317000;
    u8 privateHeader[0x50] = { 0xFF };
    u32 cartId = 0;
    char filename[64];
    
    
    // check if cartridge inserted
    if (REG_CARDCONF2 & 0x1) {
        Debug("Cartridge was not detected");
        return 1;
    }
    
    // initialize cartridge
    Cart_Init();
    cartId = Cart_GetID();
    Debug("Cartridge ID: %08X", cartId);
    *(u32*) (privateHeader + 0x40) = cartId;
    *(u32*) (privateHeader + 0x44) = 0x00000000;
    *(u32*) (privateHeader + 0x48) = 0xFFFFFFFF;
    *(u32*) (privateHeader + 0x4C) = 0xFFFFFFFF;
    
    // check for NTR cartridge
    if (!(cartId & 0x10000000)) {
        Debug("Error: NTR carts have no private headers");
        return 1;
    }
    
    // read cartridge NCCH header
    CTR_CmdReadHeader(ncch);
    if (memcmp(ncch->magic, "NCCH", 4) != 0) {
        Debug("Error reading cart NCCH header");
        return 1;
    }

    // secure init
    u32 sec_keys[4];
    Cart_Secure_Init((u32*) ncch, sec_keys);
    
    // get private header
    CTR_CmdReadUniqueID(privateHeader);
    Debug("Unique ID:");
    Debug("%016llX%016llX", getbe64(privateHeader), getbe64(privateHeader + 0x08));
    
    // dump to file
    snprintf(filename, 64, "/%s%s%.16s-private.bin", GetGameDir() ? GetGameDir() : "",
        GetGameDir() ? "/" : "", ncch->productcode);
    if (FileDumpData(filename, privateHeader, 0x50) != 0x50) {
        Debug("Could not create output file on SD");
        return 1;
    }

    return 0;
}

u32 ProcessCartSave(u32 param)
{
    u8* buffer = BUFFER_ADDRESS;
    char filename[64];
    u32 saveSize = 0;
    int saveType = 0;
    u32 cartId;
    
    // check if cartridge inserted
    if (REG_CARDCONF2 & 0x1) {
        Debug("Cartridge was not detected");
        return 1;
    }

    // initialize cartridge
    Cart_Init();
    cartId = Cart_GetID();
    Debug("Cartridge ID: %08X", Cart_GetID());
    Debug("Cartridge type: %s", (cartId & 0x10000000) ? "CTR" : "NTR/TWL");

    // check options vs. cartridge type
    if (cartId & 0x10000000) {
        Debug("Not possible for CTR carts");
        return 1;
    }
    
    // preparations
    saveType = cardEepromGetType();
    saveSize = cardEepromGetSize();
    Debug("eeprom type: %i", saveType);
    Debug("eeprom size: %u kB", saveSize / 1024);
    if ((saveType <= 0) || (saveSize > BUFFER_MAX_SIZE)) {
        Debug("Invalid eeprom chip detected or not available");
        return 1;
    }
    Debug("");
    
    if (param & CD_FLASH) {
        // user file select, load & write
        if (InputFileNameSelector(filename, "ndscart.sav", NULL, NULL, 0, saveSize, true) != 0)
            return 1;
        if (FileGetData(filename, buffer, saveSize, 0) != saveSize) {
            Debug("Error reading file");
            return 1;
        }
        Debug("Writing savegame...");
        // full erase for eeprom type 3
        if (saveType == 3) cardEepromChipErase();
        cardWriteEeprom(0, buffer, saveSize, saveType);
    } else {
        Debug("Reading savegame...");
        cardReadEeprom(0, buffer, saveSize, saveType);
        if (OutputFileNameSelector(filename, "ndscart.sav", NULL) != 0)
            return 1;
        if (FileDumpData(filename, buffer, saveSize) != saveSize) {
            Debug("Error writing file");
            return 1;
        }
    }
    
    
    return 0;
}
