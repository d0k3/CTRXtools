#pragma once

#include "common.h"
#include "decryptor/decryptor.h"

#define GC_NCCH_PROCESS (1<<0)
#define GC_CIA_PROCESS  (1<<1)
#define GC_CIA_DEEP     (1<<2)
#define GC_NCCH_ENC0x2C (1<<3)
#define GC_NCCH_ENCZERO (1<<4)
#define GC_CIA_ENCRYPT  (1<<5)
#define GC_CXI_ONLY     (1<<6)
#define GC_BOSS_PROCESS (1<<7)
#define GC_BOSS_ENCRYPT (1<<8)

#define CD_TRIM         (1<<0)
#define CD_DECRYPT      (1<<1)
#define CD_MAKECIA      (1<<2)
#define CD_FLASH        (1<<3)

#define MAX_ENTRIES 1024
#define CIA_CERT_SIZE 0xA00

typedef struct {
    u64 titleId;
    u8 external_seed[16];
    u8 reserved[8];
} __attribute__((packed)) SeedInfoEntry;

typedef struct {
    u32 n_entries;
    u8 padding[12];
    SeedInfoEntry entries[MAX_ENTRIES];
} __attribute__((packed)) SeedInfo;

typedef struct {
	u32 offset;
	u32 size;
} __attribute__((packed)) NcchPartition;

typedef struct {
	u8  signature[0x100];
	u8  magic[4];
	u32 size;
	u64 mediaId;
	u8  partitions_fs_type[8];
	u8  partitions_crypto_type[8];
	NcchPartition partitions[8];
	u8  hash_exthdr[0x20];
	u8  size_addhdr[0x4];
	u8  sector_zero_offset[0x4];
	u8  partition_flags[8];
	u8  partitionId_table[8][8];
	u8  reserved[0x30];
} __attribute__((packed, aligned(16))) NcsdHeader;

typedef struct {
    u8  signature[0x100];
    u8  magic[0x4];
    u32 size;
    u64 partitionId;
    u16 makercode;
    u16 version;
    u8  hash_seed[0x4];
    u64 programId;
    u8  reserved0[0x10];
    u8  hash_logo[0x20];
    char productcode[0x10];
    u8  hash_exthdr[0x20];
    u32 size_exthdr;
    u8  reserved1[0x4];
    u8  flags[0x8];
    u32 offset_plain;
    u32 size_plain;
    u32 offset_logo;
    u32 size_logo;
    u32 offset_exefs;
    u32 size_exefs;
    u32 size_exefs_hash;
    u8  reserved2[0x4];
    u32 offset_romfs;
    u32 size_romfs;
    u32 size_romfs_hash;
    u8  reserved3[0x4];
    u8  hash_exefs[0x20];
    u8  hash_romfs[0x20];
} __attribute__((packed, aligned(16))) NcchHeader;

// see: https://www.3dbrew.org/wiki/CIA#Meta
typedef struct {
	u8  dependencies[0x180]; // from ExtHeader
    u8  reserved0[0x180];
    u32 core_version; // 2 normally
    u8  reserved1[0xFC];
    u8  smdh[0x36C0]; // from ExeFS
} __attribute__((packed)) CiaMeta;

// from: https://github.com/profi200/Project_CTR/blob/02159e17ee225de3f7c46ca195ff0f9ba3b3d3e4/ctrtool/tik.h#L15-L39
typedef struct {
    u8 sig_type[4];
	u8 signature[0x100];
	u8 padding1[0x3C];
	u8 issuer[0x40];
	u8 ecdsa[0x3C];
    u8 version;
    u8 ca_crl_version;
    u8 signer_crl_version;
	u8 titlekey[0x10];
	u8 reserved0;
	u8 ticket_id[8];
	u8 console_id[4];
	u8 title_id[8];
	u8 sys_access[2];
	u8 ticket_version[2];
	u8 time_mask[4];
	u8 permit_mask[4];
	u8 title_export;
	u8 commonkey_idx;
    u8 reserved1[0x2A];
    u8 eshop_id[4];
    u8 reserved2;
    u8 audit;
	u8 content_permissions[0x40];
	u8 reserved3[2];
	u8 timelimits[0x40];
    u8 content_index[0xAC];
} __attribute__((packed)) Ticket;

// from: https://github.com/profi200/Project_CTR/blob/02159e17ee225de3f7c46ca195ff0f9ba3b3d3e4/ctrtool/tmd.h#L18-L59;
typedef struct {
	u8 id[4];
	u8 index[2];
    u8 type[2];
    u8 size[8];
    u8 hash[0x20];
} __attribute__((packed)) TmdContentChunk;

typedef struct {
	u8 index[2];
	u8 cmd_count[2];
	u8 hash[0x20];
} __attribute__((packed)) TmdContentInfo;

typedef struct {
    u8 sig_type[4];
    u8 signature[0x100];
	u8 padding[0x3C];
	u8 issuer[0x40];
    u8 version;
    u8 ca_crl_version;
    u8 signer_crl_version;
	u8 reserved0;
	u8 system_version[8];
	u8 title_id[8];
	u8 title_type[4];
	u8 group_id[2];
	u8 save_size[4];
	u8 twl_privsave_size[4];
	u8 reserved1[4];
	u8 twl_flag;
	u8 reserved2[0x31];
	u8 access_rights[4];
	u8 title_version[2];
	u8 content_count[2];
	u8 boot_content[2];
	u8 reserved3[2];
	u8 contentinfo_hash[0x20];
	TmdContentInfo contentinfo[64];
} __attribute__((packed)) TitleMetaData;

typedef struct {
    u32 offset_cert;
    u32 offset_ticktmd;
    u32 offset_ticket;
    u32 offset_tmd;
    u32 offset_content_list;
    u32 offset_meta;
    u32 offset_content;
    u32 size_cert;
    u32 size_ticktmd;
    u32 size_ticket;
    u32 size_tmd;
    u32 size_content_list;
    u32 size_meta;
    u64 size_content;
    u64 size_cia;
} __attribute__((packed)) CiaInfo;

typedef struct {
    u32 size_header;
    u16 type;
    u16 version;
    u32 size_cert;
    u32 size_ticket;
    u32 size_tmd;
    u32 size_meta;
    u64 size_content;
    u8  content_index[0x2000];
} __attribute__((packed)) CiaHeader;

u32 GetSdCtr(u8* ctr, const char* path);
u32 GetNcchCtr(u8* ctr, NcchHeader* ncch, u8 sub_id);
u32 SdFolderSelector(char* path, u8* keyY, bool title_select);
u32 CryptSdToSd(const char* filename, u32 offset, u32 size, CryptBufferInfo* info, bool handle_offset16);
u32 CryptNcch(const char* filename, u32 offset, u32 size, u64 seedId, u8* encrypt_flags);
u32 CryptCia(const char* filename, u8* ncch_crypt, bool cia_encrypt, bool cxi_only);
u32 CryptBoss(const char* filename, bool encrypt);

// --> FEATURE FUNCTIONS <--
u32 CryptGameFiles(u32 param);
u32 ConvertNcsdNcchToCia(u32 param);
u32 CryptSdFiles(u32 param);
u32 DecryptSdFilesDirect(u32 param);
u32 ConvertSdToCia(u32 param);
u32 DecryptSdToCxi(u32 param);
u32 DumpGameCart(u32 param);
u32 DumpPrivateHeader(u32 param);
u32 ProcessCartSave(u32 param);
