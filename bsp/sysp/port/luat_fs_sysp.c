#include "luat_base.h"
#include "luat_fs.h"
#include "luat_malloc.h"

#ifdef LUAT_USE_LVGL
#include "lvgl.h"
void luat_lv_fs_init(void);
void lv_bmp_init(void);
void lv_png_init(void);
void lv_split_jpeg_init(void);
#endif

extern const struct luat_vfs_filesystem vfs_fs_posix;
extern const struct luat_vfs_filesystem vfs_fs_onefile;

// #ifdef LUAT_USE_VFS_INLINE_LIB
extern const char luat_inline_sys[];
extern const uint32_t luat_inline_sys_size;
// #endif

typedef struct luat_fs_onefile
{
    char* ptr;
    uint32_t  size;
    uint32_t  offset;
}luat_fs_onefile_t;

static uint8_t fs_init_done = 0;

int luat_fs_init(void) {
	if (fs_init_done)
		return 0;
	fs_init_done = 1;

	#ifdef LUAT_USE_FS_VFS
	// vfs进行必要的初始化
	luat_vfs_init(NULL);
	// 注册vfs for posix 实现
	luat_vfs_reg(&vfs_fs_posix);
	luat_vfs_reg(&vfs_fs_onefile);

	luat_fs_conf_t conf = {
		.busname = "",
		.type = "posix",
		.filesystem = "posix",
		.mount_point = "/", // window环境下, 需要支持任意路径的读取,不能强制要求必须是/
	};
	luat_fs_mount(&conf);
	// #ifdef LUAT_USE_VFS_INLINE_LIB
    luat_fs_onefile_t* fd = luat_heap_malloc(sizeof(luat_fs_onefile_t));
    fd->ptr = luat_inline_sys;
    fd->size = luat_inline_sys_size;
	luat_fs_conf_t conf2 = {
		.busname = (char*)fd,
		.type = "onefile",
		.filesystem = "onefile",
		.mount_point = "/luadb/sys.luac",
	};
	luat_fs_mount(&conf2);

	// FILE* fd2 = luat_fs_fopen("/luadb/sys.lua", "rb");
	// char buff[1024];
	// luat_fs_fread(buff, 1, 1024, fd2);
	// #endif
	#endif

	#ifdef LUAT_USE_LVGL
	luat_lv_fs_init();
	lv_bmp_init();
	lv_png_init();
	lv_split_jpeg_init();
	#endif
	return 0;
}
