
#ifndef LUAT_CONF_BSP
#define LUAT_CONF_BSP

// 启用64位虚拟机
// #define LUAT_CONF_VM_64bit

//#define LUAT_CONF_LUASTATE_NOT_STATIC

// #define LUAT_CONF_LAUX_BUFFSIZE 1024

// #define LUA_COMPAT_BITLIB 1

//#define LUAT_CONF_DISABLE_ROTABLE

// #define LUAT_USE_FS_VFS 1

// #define LUAT_USE_VFS_INLINE_LIB 1

// #define LUAT_MEMORY_OPT_G_FUNCS 1


// #define LUAT_USE_LVGL 1

#define LUAT_FS_NO_POSIX_GETC


#define luai_makeseed(x)    (0x12345678)
#endif
