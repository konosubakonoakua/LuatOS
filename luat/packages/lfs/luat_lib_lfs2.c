

#include "luat_base.h"
#include "luat_sfd.h"
#include "luat_spi.h"
#include "luat_gpio.h"
#include "luat_malloc.h"

#include "lfs.h"

int lfs_sfd_read(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size);
int lfs_sfd_prog(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size);
int lfs_sfd_erase(const struct lfs_config *c, lfs_block_t block);
int lfs_sfd_sync(const struct lfs_config *c);

typedef struct lfs2_mount {
    char path[16];
    void* userdata;
    int luaref;
    lfs_t* fs;
}lfs2_mount_t;

static lfs2_mount_t mounted[2] = {0};

static int l_lfs2_mount(lua_State *L) {
    const char* path = luaL_checkstring(L, 1);
    sfd_w25q_t *w25q = lua_touserdata(L, 2);
    for (size_t i = 0; i < 2; i++)
    {
        if (mounted[i].userdata == NULL) {
            mounted[i].userdata = w25q;
            memcpy(mounted[i].path, path, strlen(path) + 1);
            
            lua_settop(L, 2);
            mounted[i].luaref = luaL_ref(L, LUA_REGISTRYINDEX);
            
            mounted[i].fs = luat_heap_malloc(sizeof(lfs_t));
            struct lfs_config* cfg = (struct lfs_config*)luat_heap_malloc(sizeof(struct lfs_config));

            memset(cfg, 0, sizeof(struct lfs_config));
            memset(mounted[i].fs, 0, sizeof(lfs_t));

            cfg->read  = lfs_sfd_read;
            cfg->prog  = lfs_sfd_prog;
            cfg->erase = lfs_sfd_erase;
            cfg->sync  = lfs_sfd_sync;

            cfg->read_size = 256;
            cfg->prog_size = 256;
            cfg->block_size = 4096;
            cfg->block_count = w25q->sector_count / 16;
            cfg->block_cycles = 200;
            cfg->cache_size = 16;
            cfg->lookahead_size = 256;

            // cfg.read_buffer = lfs_read_buf,
            // cfg.prog_buffer = lfs_prog_buf,
            // cfg.lookahead_buffer = lfs_lookahead_buf,
            cfg->name_max = 63;
            cfg->file_max = 0;
            cfg->attr_max = 0;

            cfg->context = w25q;

            int ret = lfs_mount(mounted[i].fs, cfg);
            if (ret)
                LLOGW("lfs_mount ret %d", ret);
            lua_pushboolean(L, ret == 0 ? 1 : 0);
            return 1;
        }
    }
    return 0;
}


#include "rotable.h"
static const rotable_Reg reg_lfs2[] =
{ 
  { "mount",	l_lfs2_mount, 0}, //初始化,挂载
//   { "unmount",	l_lfs2_unmount, 0}, // 取消挂载
//   { "mkfs",		l_lfs2_mkfs, 0}, // 格式化!!!
//   //{ "test",  l_lfs2_test, 0},
//   { "getfree",	l_lfs2_getfree, 0}, // 获取文件系统大小,剩余空间
//   { "debug",	l_lfs2_debug_mode, 0}, // 调试模式,打印更多日志

//   { "lsdir",	l_lfs2_lsdir, 0}, // 列举目录下的文件,名称,大小,日期,属性
//   { "mkdir",	l_lfs2_mkdir, 0}, // 列举目录下的文件,名称,大小,日期,属性

//   { "stat",		l_lfs2_stat, 0}, // 查询文件信息
//   { "open",		l_lfs2_open, 0}, // 打开一个文件句柄
//   { "close",	l_lfs2_close, 0}, // 关闭一个文件句柄
//   { "seek",		l_lfs2_seek, 0}, // 移动句柄的当前位置
//   { "truncate",	l_lfs2_truncate, 0}, // 缩减文件尺寸到当前seek位置
//   { "read",		l_lfs2_read, 0}, // 读取数据
//   { "write",	l_lfs2_write, 0}, // 写入数据
//   { "remove",	l_lfs2_remove, 0}, // 删除文件,别名方法
//   { "unlink",	l_lfs2_remove, 0}, // 删除文件
//   { "rename",	l_lfs2_rename, 0}, // 文件改名

//   { "readfile",	l_lfs2_readfile, 0}, // 读取文件的简易方法
//   { "playmp3",	l_lfs2_playmp3, 0}, // 读取文件的简易方法
  { NULL,		NULL,	0 }
};

int luaopen_lfs2( lua_State *L )
{
  luat_newlib(L, reg_lfs2);
  return 1;
}
