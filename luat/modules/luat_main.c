

#include "luat_base.h"
#include "luat_malloc.h"
#include "luat_fs.h"
#include "stdio.h"
#include "luat_msgbus.h"
#include "luat_timer.h"

#define LUAT_LOG_TAG "luat.main"
#include "luat_log.h"

#ifdef LUA_USE_WINDOWS
extern int win32_argc;
extern char** win32_argv;
#endif

static int report (lua_State *L, int status);

lua_State *L;

static uint8_t boot_mode = 1;

void stopboot(void) {
  boot_mode = 0;
}

lua_State * luat_get_state() {
  return L;
}

static int pmain(lua_State *L) {
    int re = 0;

    #ifdef LUA_32BITS
    //LLOGD("Lua complied with LUA_32BITS");
    #endif

    // 加载内置库
    luat_openlibs(L);

    size_t total; size_t used; size_t max_used;
    luat_meminfo_luavm(&total, &used, &max_used);
    LLOGD("luavm %ld %ld %ld", total, used, max_used);
    luat_meminfo_sys(&total, &used, &max_used);
    LLOGD("sys   %ld %ld %ld", total, used, max_used);

    lua_gc(L, LUA_GCSETPAUSE, 90); // 设置`垃圾收集器间歇率`要低于100%
    
    // 加载main.lua
    #ifdef LUA_USE_WINDOWS
    if (win32_argc > 1) {
      int slen = strlen(win32_argv[1]);
      if (slen > 4 && !strcmp(".lua", win32_argv[1] + (slen - 4))) {
        re = luaL_dofile(L, win32_argv[1]);
      }
      else {
        re = luaL_dostring(L, "require(\"main\")");
      }
    }
    #else
    re = luaL_dostring(L, "require(\"main\")");
    #endif

    report(L, re);
    lua_pushboolean(L, 1);  /* signal no errors */
    return 1;
}

/*
** Prints an error message, adding the program name in front of it
** (if present)
*/
static void l_message (const char *pname, const char *msg) {
  if (pname) lua_writestringerror("%s: ", pname);
  lua_writestringerror("%s\n", msg);
}


/*
** Check whether 'status' is not OK and, if so, prints the error
** message on the top of the stack. It assumes that the error object
** is a string, as it was either generated by Lua or by 'msghandler'.
*/
static int report (lua_State *L, int status) {
  if (status != LUA_OK) {
    const char *msg = lua_tostring(L, -1);
    l_message("LUAT", msg);
    lua_pop(L, 1);  /* remove message */
  }
  return status;
}

static int panic (lua_State *L) {
  lua_writestringerror("PANIC: unprotected error in call to Lua API (%s)\n",
                        lua_tostring(L, -1));
  return 0;  /* return to Lua to abort */
}

#define UPDATE_BIN_PATH "/update.bin"
#define ROLLBACK_MARK_PATH "/rollback_mark"
#define UPDATE_MARK "/update_mark"
#define FLASHX_PATH "/flashx.bin"

int luat_bin_unpack(const char* path, int writeOut);

static void check_update(void) {
  // 首先, 升级文件是否存在呢?
  if (!luat_fs_fexist(UPDATE_BIN_PATH)) {
    // 不存在, 正常启动
    return;
  }
  // 找到了, 检查一下大小
  LLOGI("found " UPDATE_BIN_PATH " ...");
  size_t fsize = luat_fs_fsize(UPDATE_BIN_PATH);
  if (fsize < 256) {
    // 太小了, 肯定不合法, 直接移除, 正常启动
    LLOGW(UPDATE_BIN_PATH " is too small, delete it");
    luat_fs_remove(UPDATE_BIN_PATH);
    return;
  }
  // 写入标志文件.
  // 必须提前写入, 即使解包失败, 仍标记为升级过,这样报错就能回滚
  LLOGI("write " UPDATE_MARK  " first");
  FILE* fd = luat_fs_fopen(UPDATE_MARK, "wb");
  if (fd) {
    luat_fs_fclose(fd);
    // TODO 连标志文件都写入失败,怎么办?
  }
  // 检测升级包合法性
  if (luat_bin_unpack(UPDATE_BIN_PATH, 0) != LUA_OK) {
    LLOGE("%s is invaild!!", UPDATE_BIN_PATH); 
  }
  else {
    // 开始解包升级文件
    if (luat_bin_unpack(UPDATE_BIN_PATH, 1) == LUA_OK) {
      LLOGI("update OK, remove " UPDATE_BIN_PATH);
    }
    else {
      LLOGW("update FAIL, remove " UPDATE_BIN_PATH);
    }
  }
  // 无论是否成功,都一定要删除升级文件, 防止升级死循环
  luat_fs_remove(UPDATE_BIN_PATH);
  // 延迟5秒,重启
  LLOGW("update: reboot at 5 secs");
  luat_timer_mdelay(5*1000);
  luat_os_reboot(0); // 重启
}

static void check_rollback(void) {
  // 首先, 查一下是否有回滚文件
  if (!luat_fs_fexist(ROLLBACK_MARK_PATH)) {
    return; // 没有回滚标志文件, 正常启动
  }
  // 回滚文件存在,
  LLOGW("Found " ROLLBACK_MARK_PATH  ", check rollback");
  // 首先,移除回滚标志, 防止重复N次的回滚
  luat_fs_remove("/rollback_mark"); // TODO 如果删除也失败呢?
  // 然后检查原始文件, flashx.bin
  if (!luat_fs_fexist(FLASHX_PATH)) {
    LLOGW("NOT " FLASHX_PATH " , can't rollback");
    return;
  }
  // 存在原始flashx.bin
  LLOGD("found " FLASHX_PATH  ", unpack it");
  // 开始回滚操作
  if (luat_bin_unpack(FLASHX_PATH, 1) == LUA_OK) {
    LLOGI("rollback complete!");
  }
  else {
    LLOGE("rollback FAIL");
  }
  // 执行完成, 准备重启
  LLOGW("rollback: reboot at 5 secs");
  // 延迟5秒后,重启
  luat_timer_mdelay(5*1000);
  luat_os_reboot(0); // 重启
}

int luat_main (void) {
  if (boot_mode == 0) {
    return 0; // just nop
  }
  LLOGI("LuatOS@%s %s, Build: " __DATE__ " " __TIME__, luat_os_bsp(), LUAT_VERSION);
  #if LUAT_VERSION_BETA
  LLOGD("This is a beta version, for testing");
  #endif
  // 1. 初始化文件系统
  luat_fs_init();

  // 2. 是否需要升级?
  check_update();

  // 3. 是否需要回滚呢?
  check_rollback();

  // 4. init Lua State
  int status, result;
  L = lua_newstate(luat_heap_alloc, NULL);
  if (L == NULL) {
    l_message("LUAVM", "cannot create state: not enough memory\n");
    goto _exit;
  }
  if (L) lua_atpanic(L, &panic);
  //print_list_mem("after lua_newstate");
  lua_pushcfunction(L, &pmain);  /* to call 'pmain' in protected mode */
  //lua_pushinteger(L, argc);  /* 1st argument */
  //lua_pushlightuserdata(L, argv); /* 2nd argument */
  status = lua_pcall(L, 0, 1, 0);  /* do the call */
  result = lua_toboolean(L, -1);  /* get result */
  report(L, status);
  //lua_close(L);
_exit:
  LLOGE("Lua VM exit!! reboot in 30s");
  // 既然是异常退出,那肯定出错了!!!
  // 如果升级过, 那么就写入标志文件
  {
    if (luat_fs_fexist(UPDATE_MARK)) {
      FILE* fd = luat_fs_fopen("/rollback_mark", "wb");
      if (fd) {
        luat_fs_fclose(fd);
      }
    }
    else {
      // 没升级过, 那就是线刷, 不存在回滚
    }
  }
  // 等30秒,就重启吧
  luat_timer_mdelay(30*1000);
  luat_os_reboot(result);
  // 往下是肯定不会被执行的
  return (result && status == LUA_OK) ? 0 : 2;
}
