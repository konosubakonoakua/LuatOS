

#include "luat_base.h"
#include "luat_malloc.h"
#include "luat_fs.h"
#include "stdio.h"
#include "luat_msgbus.h"
#include "luat_timer.h"
#include "luat_rtos.h"

#include "luat_ota.h"

#define LUAT_LOG_TAG "main"
#include "luat_log.h"

#ifdef LUAT_USE_ERRDUMP
#include "luat_errdump.h"
#endif

#ifdef LUAT_USE_PROFILER
#include "luat_profiler.h"
#endif

#ifdef LUAT_USE_WDT
#include "luat_wdt.h"
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

luat_rtos_timer_t luar_error_timer;

static LUAT_RT_RET_TYPE l_timer_error_cb(LUAT_RT_CB_PARAM) {
  LLOGE("未找到main.lua,请刷入脚本以运行程序,luatos快速入门教程: https://wiki.luatos.com/boardGuide/roadmap.html");
  LLOGE("The main.lua not found, please flash the script to run the program, luatos quick start: https://wiki.luatos.com/boardGuide/roadmap.html");
}

int luat_search_module(const char* name, char* filename);
void luat_os_print_heapinfo(const char* tag);
void luat_force_gc_all(void)
{
	lua_gc(L, LUA_GCCOLLECT, 0);
}
int luat_main_demo() { // 这是验证LuatVM最基础的消息/定时器/Task机制是否正常
  return luaL_dostring(L, "local sys = require \"sys\"\n"
                          "log.info(\"main\", os.date())\n"
                          "leda = gpio.setup(3, 0)"
                          "sys.taskInit(function ()\n"
                          "  while true do\n"
                          "    log.info(\"hi\", rtos.meminfo())\n"
                          "    sys.wait(500)\n"
                          "    leda(1)\n"
                          "    sys.wait(500)\n"
                          "    leda(0)\n"
                          "    log.info(\"main\", os.date())\n"
                          "  end\n"
                          "end)\n"
                          "sys.run()\n");
}

static int pmain(lua_State *L) {
    int re = -2;
    char filename[32] = {0};

    //luat_os_print_heapinfo("boot");

    // 加载内置库
    luat_openlibs(L);

    luat_os_print_heapinfo("loadlibs");

    lua_gc(L, LUA_GCSETPAUSE, 90); // 设置`垃圾收集器间歇率`要低于100%

#ifdef LUAT_HAS_CUSTOM_LIB_INIT
    luat_custom_init(L);
#endif
    if (re == -2) {
      #ifndef LUAT_MAIN_DEMO
        if (luat_search_module("main", filename) == 0) {
          re = luaL_dofile(L, filename);
        }
        else {
          re = -1;
          luar_error_timer = luat_create_rtos_timer(l_timer_error_cb, NULL, NULL);
          luat_start_rtos_timer(luar_error_timer, 1000, 1);
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
#ifdef LUAT_LOG_NO_NEWLINE
  LLOGE("%s", strlen(msg), msg);
#else
  LLOGE("%s\n", msg);
#endif
}


/*
** Check whether 'status' is not OK and, if so, prints the error
** message on the top of the stack. It assumes that the error object
** is a string, as it was either generated by Lua or by 'msghandler'.
*/
static int report (lua_State *L, int status) {
  size_t len = 0;
  if (status != LUA_OK) {
    const char *msg = lua_tolstring(L, -1, &len);
    //luaL_traceback(L, L, msg, 1);
    //msg = lua_tolstring(L, -1, &len);
    LLOGE("Luat: ");
    LLOGE("%s", msg);
    // LLOGD("MSG2 ==> %s %d", msg, len);
    // l_message("LUAT", msg);
#ifdef LUAT_USE_ERRDUMP
    luat_errdump_save_file(msg, strlen(msg));
#endif
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
#ifdef LUAT_USE_PROFILER
  L = lua_newstate(luat_profiler_alloc, NULL);
#else
  L = lua_newstate(luat_heap_alloc, NULL);
#endif
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
  #ifdef LUAT_CONF_VM_64bit
  LLOGI("LuatOS@%s base %s bsp %s 64bit", luat_os_bsp(), LUAT_VERSION, LUAT_BSP_VERSION);
  #else
  LLOGI("LuatOS@%s base %s bsp %s 32bit", luat_os_bsp(), LUAT_VERSION, LUAT_BSP_VERSION);
  #endif
  LLOGI("ROM Build: " __DATE__ " " __TIME__);
  // #if LUAT_VERSION_BETA
  // LLOGD("This is a beta/snapshot version, for testing");
  // #endif
  #else
  #ifdef LUAT_CONF_VM_64bit
  LLOGI("LuatOS@%s %s, Build: " __DATE__ " " __TIME__ " 64bit", luat_os_bsp(), LUAT_VERSION);
  #else
  LLOGI("LuatOS@%s %s, Build: " __DATE__ " " __TIME__ " 32bit", luat_os_bsp(), LUAT_VERSION);
  #endif
  #if LUAT_VERSION_BETA
  LLOGD("This is a beta version, for testing");
  #endif
  #endif

  // 1. 初始化文件系统
  luat_fs_init();
#ifdef LUAT_USE_OTA
  if (luat_ota_exec() == 0) {
    luat_os_reboot(5);
  }
#endif
  luat_main_call();
  LLOGE("Lua VM exit!! reboot in %dms", LUAT_EXIT_REBOOT_DELAY);
#ifdef LUAT_USE_WDT
  for (size_t i = 1; i < LUAT_EXIT_REBOOT_DELAY / 1000; i++)
  {
    luat_wdt_feed();
    luat_timer_mdelay(1000);
  }
  luat_ota_reboot(1000);
#else
  luat_ota_reboot(LUAT_EXIT_REBOOT_DELAY);
#endif
  // 往下是肯定不会被执行的
  return 0;
}

