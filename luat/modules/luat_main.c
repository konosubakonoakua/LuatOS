

#include "luat_base.h"
#include "luat_malloc.h"
#include "luat_fs.h"
#include "stdio.h"
#include "luat_msgbus.h"
#include "luat_timer.h"

#include "luat_ota.h"

#define LUAT_LOG_TAG "luat.main"
#include "luat_log.h"

#ifndef LUAT_USE_CMDLINE_ARGS
#ifdef LUA_USE_WINDOWS
#define LUAT_USE_CMDLINE_ARGS 1
#endif
#ifdef LUA_USE_LINUX
#define LUAT_USE_CMDLINE_ARGS 1
#endif
#endif

#ifdef LUAT_USE_CMDLINE_ARGS
#include <stdlib.h>
extern int cmdline_argc;
extern char** cmdline_argv;
#endif

static int report (lua_State *L, int status);

lua_State *L;

static uint8_t boot_mode = 1;

void stopboot(void) {
  boot_mode = 0;
}

// lua_State * luat_get_state() {
//   return L;
// }

int luat_search_module(const char* name, char* filename);
void luat_os_print_heapinfo(const char* tag);

int luat_main_demo() { // 这是验证LuatVM最基础的消息/定时器/Task机制是否正常
  return luaL_dostring(L, "local sys = require \"sys\"\n"
                          "log.info(\"main\", os.date())\n"
                          "led = gpio.setup(19, 0)"
                          "sys.taskInit(function ()\n"
                          "  while true do\n"
                          "    log.info(\"hi\", rtos.meminfo())\n"
                          "    sys.wait(500)\n"
                          "    led(1)\n"
                          "    sys.wait(500)\n"
                          "    led(0)\n"
                          "  end\n"
                          "end)\n"
                          "sys.run()\n");
}

static int pmain(lua_State *L) {
    int re = -2;

    luat_os_print_heapinfo("boot");

    // 加载内置库
    luat_openlibs(L);

    luat_os_print_heapinfo("loadlibs");

    lua_gc(L, LUA_GCSETPAUSE, 90); // 设置`垃圾收集器间歇率`要低于100%

    // 加载main.lua
    #ifdef LUAT_USE_CMDLINE_ARGS
    if (cmdline_argc > 1) {
      int slen = strlen(cmdline_argv[1]);
      if (slen > 4 && !strcmp(".lua", cmdline_argv[1] + (slen - 4)))
        re = luaL_dofile(L, cmdline_argv[1]);
    }
    #endif
    if (re == -2) {
      #ifndef LUAT_MAIN_DEMO
      char filename[32] = {0};
      if (luat_search_module("main", filename) == 0) {
        re = luaL_dofile(L, filename);
      }
      else {
        re = -1;
        luaL_error(L, "module '%s' not found", "main");
      }
      #else
      re = luat_main_demo();
      #endif
    }
        
    report(L, re);
    lua_pushboolean(L, re == LUA_OK);  /* signal no errors */
    return 1;
}

/*
** Prints an error message, adding the program name in front of it
** (if present)
*/
static void l_message (const char *pname, const char *msg) {
  if (pname) LLOGE("%s: ", pname);
  LLOGE("%s\n", msg);
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
  LLOGE("PANIC: unprotected error in call to Lua API (%s)\n",
                        lua_tostring(L, -1));
  return 0;  /* return to Lua to abort */
}


int luat_main_call(void) {
  // 4. init Lua State
  int status = 0;
  int result = 0;
  L = lua_newstate(luat_heap_alloc, NULL);
  if (L == NULL) {
    l_message("lua", "cannot create state: not enough memory\n");
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
  #ifdef LUAT_USE_CMDLINE_ARGS
    result = !result;
    LLOGE("Lua VM exit!! result:%d",result);
    exit(result);
  #endif
  return result;
}

/**
 * 常规流程, 单一入口, 执行脚本.
 * require "sys"
 * 
 * ... 用户代码 ....
 * 
 * sys.run() 
*/
int luat_main (void) {
  if (boot_mode == 0) {
    return 0; // just nop
  }
  #ifdef LUAT_BSP_VERSION
  LLOGI("LuatOS@%s core %s bsp %s", luat_os_bsp(), LUAT_VERSION, LUAT_BSP_VERSION);
  LLOGI("ROM Build: " __DATE__ " " __TIME__);
  #else
  LLOGI("LuatOS@%s %s, Build: " __DATE__ " " __TIME__, luat_os_bsp(), LUAT_VERSION);
  #if LUAT_VERSION_BETA
  LLOGD("This is a beta version, for testing");
  #endif
  #endif
  

  // 1. 初始化文件系统
  luat_fs_init();

  // 是否需要升级或者回滚
  luat_ota_update_or_rollback();

  int result = luat_main_call();
  LLOGE("Lua VM exit!! reboot in %dms", LUAT_EXIT_REBOOT_DELAY);
  luat_ota_reboot(LUAT_EXIT_REBOOT_DELAY);
  // 往下是肯定不会被执行的
  return 0;
}



#include "rotable.h"
void luat_newlib(lua_State* l, const rotable_Reg* reg) {
  #ifdef LUAT_CONF_DISABLE_ROTABLE
  luaL_newlibtable(l,reg);
  int i;
  int nup = 0;

  luaL_checkstack(l, nup, "too many upvalues");
  for (; reg->name != NULL; reg++) {  /* fill the table with given functions */
        for (i = 0; i < nup; i++)  /* copy upvalues to the top */
            lua_pushvalue(l, -nup);
        if (reg->func)
            lua_pushcclosure(l, reg->func, nup);  /* closure with those upvalues */
        else
            lua_pushinteger(l, reg->value);
        lua_setfield(l, -(nup + 2), reg->name);
    }
    lua_pop(l, nup);  /* remove upvalues */
  #else
  rotable_newlib(l, reg);
  #endif
}

void luat_os_print_heapinfo(const char* tag) {
    size_t total; size_t used; size_t max_used;
    luat_meminfo_luavm(&total, &used, &max_used);
    LLOGD("%s luavm %ld %ld %ld", tag, total, used, max_used);
    luat_meminfo_sys(&total, &used, &max_used);
    LLOGD("%s sys   %ld %ld %ld", tag, total, used, max_used);
}
