
#include "luat_base.h"
#include "luat_fs.h"

#ifdef LUAT_USE_SFUD
#ifdef LUAT_USE_FS_VFS
#include "lfs.h"
#include "sfud.h"
#define LFS_BLOCK_SIZE (4096)
#define LFS_BLOCK_DEVICE_READ_SIZE (256)
#define LFS_BLOCK_DEVICE_PROG_SIZE (256)
#define LFS_BLOCK_DEVICE_CACHE_SIZE (256)
#define LFS_BLOCK_DEVICE_ERASE_SIZE (4096) // one sector 4KB
//#define LFS_BLOCK_DEVICE_TOTOAL_SIZE (FLASH_FS_REGION_SIZE)
#define LFS_BLOCK_DEVICE_LOOK_AHEAD (16)

// Read a block
static int block_device_read(const struct lfs_config *cfg, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size) {
    sfud_flash* flash = (sfud_flash*)cfg->context;
    sfud_read(flash, buffer, block * LFS_BLOCK_SIZE + off, size);
    return LFS_ERR_OK;
}

static int block_device_prog(const struct lfs_config *cfg, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size) {
    sfud_flash* flash = (sfud_flash*)cfg->context;
    sfud_write(flash, buffer, block * LFS_BLOCK_SIZE + off, size);
    return LFS_ERR_OK;
}

static int block_device_erase(const struct lfs_config *cfg, lfs_block_t block) {
    sfud_flash* flash = (sfud_flash*)cfg->context;
    sfud_erase(flash, block * LFS_BLOCK_SIZE, LFS_BLOCK_SIZE);
    return LFS_ERR_OK;
}

static int block_device_sync(const struct lfs_config *cfg) {
    return LFS_ERR_OK;
}

typedef struct LFS2 {
    lfs_t lfs;
    struct lfs_config cfg;
    uint8_t read_buffer[LFS_BLOCK_DEVICE_READ_SIZE];
    uint8_t prog_buffer[LFS_BLOCK_DEVICE_PROG_SIZE];
    uint8_t lookahead_buffer[LFS_BLOCK_DEVICE_LOOK_AHEAD];
}LFS2_t;


lfs_t* flash_lfs_sfud(sfud_flash* flash) {
    LFS2_t *_lfs = luat_heap_malloc(sizeof(LFS2_t));
    if (_lfs == NULL)
        return NULL;
    lfs_t *lfs = &_lfs->lfs;
    struct lfs_config *lfs_cfg = &_lfs->cfg;

    lfs_cfg->context = flash,
    // block device operations
    lfs_cfg->read = block_device_read;
    lfs_cfg->prog = block_device_prog;
    lfs_cfg->erase = block_device_erase;
    lfs_cfg->sync = block_device_sync;

    // block device configuration
    lfs_cfg->read_size = LFS_BLOCK_DEVICE_READ_SIZE;
    lfs_cfg->prog_size = LFS_BLOCK_DEVICE_PROG_SIZE;
    lfs_cfg->block_size = flash->chip.erase_gran;
    lfs_cfg->block_count = flash->chip.capacity / flash->chip.erase_gran;
    lfs_cfg->block_cycles = 200;
    lfs_cfg->cache_size = LFS_BLOCK_DEVICE_CACHE_SIZE;
    lfs_cfg->lookahead_size = LFS_BLOCK_DEVICE_LOOK_AHEAD;

    lfs_cfg->read_buffer = _lfs->read_buffer;
    lfs_cfg->prog_buffer = _lfs->prog_buffer;
    lfs_cfg->lookahead_buffer = _lfs->lookahead_buffer;
    lfs_cfg->name_max = 63;
    lfs_cfg->file_max = 0;
    lfs_cfg->attr_max = 0;

    // ------
    int err = lfs_mount(&lfs, &lfs_cfg);
    LLOGD("lfs_mount %d",err);
    if (err)
    {
        err = lfs_format(&lfs, &lfs_cfg);
        LLOGD("lfs_format %d",err);
        if(err)
            goto fail;
        err = lfs_mount(&lfs, &lfs_cfg);
        LLOGD("lfs_mount %d",err);
        if(err)
            goto fail;
    }
    return lfs;
fail :
    luat_heap_free(_lfs);
    return NULL;
    //------
}

#endif

#endif

