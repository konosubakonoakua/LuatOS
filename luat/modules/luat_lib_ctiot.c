/*
@module  ctiot
@summary coap数据处理
@version 1.0
@date    2020.06.30
*/
#include "luat_base.h"
#include "luat_timer.h"
#include "luat_malloc.h"
#include "luat_msgbus.h"
#include "luat_ctiot.h"
#define LUAT_LOG_TAG "luat.ctiot"
#include "luat_log.h"
//---------------------------
#ifdef AIR302
#define DBG(X,Y...) LLOGD("%s %d:"X, __FUNCTION__, __LINE__, ##Y)
const char luat_pub_tx_tag[] = "CTIOT_TX";
const char luat_pub_rx_tag[] = "CTIOT_RX";
const char luat_pub_reg_tag[] = "CTIOT_REG";
const char luat_pub_dereg_tag[] = "CTIOT_DEREG";
const char luat_pub_wakeup_tag[] = "CTIOT_WAKEUP";
const char luat_pub_other_tag[] = "CTIOT_OTHER";
const char luat_pub_fota_tag[] = "CTIOT_FOTA";
static int luat_ctiot_msg_handler(lua_State *L, void* ptr)
{
	uint8_t type;
	uint8_t code;
	uint32_t len;
	uint8_t *buff = (uint8_t *)ptr;
	char *tag;
	LUA_INTEGER error,error_code,param;
	type = buff[0];
	code = buff[1];
	memcpy(&len, buff+2, 4);
	switch(type)
	{
	case CTIOT_EVENT_TX:
		tag = (char *)luat_pub_tx_tag;
		param = 0;
		switch (code)
		{
		case CTIOT_TX_ACK:
		case CTIOT_TX_DONE:
			goto LUAT_CTIOT_MSG_HANDLER_DONE;
			break;
		default:
			error = 1;
			error_code = code;
			break;
		}
		break;
	case CTIOT_EVENT_AIR:
		tag = (char *)luat_pub_tx_tag;
		param = 0;
		switch (code)
		{
		case CTIOT_AIR_NON_SEND_START:
			goto LUAT_CTIOT_MSG_HANDLER_DONE;
			break;
		default:
			error = 0;
			error_code = 0;
			param = len;
			break;
		}
		break;
	case CTIOT_EVENT_RX:
		tag = (char *)luat_pub_rx_tag;
		break;
	case CTIOT_EVENT_STATE:
		switch (code)
		{
		case CTIOT_STATE_TAU_WAKEUP:
		case CTIOT_STATE_TAU_NOTIFY:
			tag = (char *)luat_pub_wakeup_tag;
			error = 0;
			error_code = 0;
			param = code;
			break;
		default:
			tag = (char *)luat_pub_other_tag;
			error = 0;
			error_code = type;
			param = code;
			break;
		}
		break;
	case CTIOT_EVENT_REG:
		tag = (char *)luat_pub_reg_tag;
		switch (code)
		{
		case CTIOT_REG_OK:
			error = 0;
			error_code = 0;
			param = 0;
			break;
		default:
			error = 1;
			error_code = code;
			param = 0;
			break;
		}
		break;
	case CTIOT_EVENT_OB19:
		tag = (char *)luat_pub_reg_tag;
		switch (code)
		{
		case CTIOT_OB19_ON:
			error = 0;
			error_code = 0;
			param = 1;
			break;
		default:
			error = 1;
			error_code = code;
			param = 0;
			break;
		}
		break;
	case CTIOT_EVENT_DEREG:
		tag = (char *)luat_pub_dereg_tag;
		error = 0;
		error_code = 0;
		param = 0;
		break;
	case CTIOT_EVENT_FOTA:
		tag = (char *)luat_pub_fota_tag;
		error = 0;
		error_code = 0;
		param = code;
		break;
	default:
		tag = (char *)luat_pub_other_tag;
		error = 0;
		error_code = type;
		param = code;
		break;
	}
	lua_getglobal(L, "sys_pub");
	lua_pushstring(L, tag);
	if (CTIOT_EVENT_RX == type)
	{
		lua_pushlstring(L, buff + 6, len);
		lua_call(L, 2, 0);
	}
	else
	{
		lua_pushboolean(L, error);
		lua_pushinteger(L, error_code);
		lua_pushinteger(L, param);
		lua_call(L, 4, 0);
	}
LUAT_CTIOT_MSG_HANDLER_DONE:
	luat_heap_free(ptr);
	return 0;
}


void luat_ctiot_callback(uint8_t type, uint8_t code, void *buf, uint32_t len)
{
	rtos_msg_t msg;
	uint8_t *buff;
	DBG("%d,%d,0x%08x,%u",type, code, buf, len);
	msg.handler = luat_ctiot_msg_handler;
	if (buf)
	{
		msg.ptr = luat_heap_malloc(6 + len);
	}
	else
	{
		msg.ptr = luat_heap_malloc(6);
	}
	buff = msg.ptr;
	if (buff)
	{
		buff[0] = type;
		buff[1] = code;
		memcpy(buff + 2, &len, 4);
		if (buf)
		{
			memcpy(buff + 6, buf, len);
		}
	}
	luat_msgbus_put(&msg, -1);

}
/**
 * @brief 初始化ctiot，在复位开机后使用一次
 * 
 * @param L 
 * @return int 
 */
static int l_ctiot_init(lua_State *L)
{
	luat_ctiot_init();
	return 0;
}

/**
 * @brief 设置和读取ctiot相关参数，有参数输入则设置，无论是否有参数输入，均输出当前参数
 * 
 * @param L 
 * @return int 
 */
static int l_ctiot_param(lua_State *L)
{
	char server_ip[20];
	const char *new_ip;
	uint16_t port;
	uint16_t result;
	uint32_t lifetime;
	size_t len;
	uint8_t new_set = 0;
	result = luat_ctiot_get_para(server_ip, &port, &lifetime, NULL);
	new_ip = lua_tolstring(L, 1, &len);
	if (len < 20 && len)
	{
		memset(server_ip, 0, 20);
		memcpy(server_ip, new_ip, len);
		new_set = 1;
	}
	if (lua_isinteger(L, 2))
	{
		port = lua_tointeger(L, 2);
		new_set = 1;
	}
	if (lua_isinteger(L, 3))
	{
		lifetime = lua_tointeger(L, 3);
		new_set = 1;
	}
	if (new_set)
	{
		result = luat_ctiot_set_para(server_ip, port, lifetime, NULL);
	}
	lua_pushstring(L, server_ip);
	lua_pushinteger(L, port);
	lua_pushinteger(L, lifetime);
	return 3;
}

/**
 * @brief 设置和读取自定义EP
 *
 * @param L
 * @return int
 */
static int l_ctiot_ep(lua_State *L)
{
	char userEp[40];
	const char *new_ep;
	size_t len;
	uint8_t new_set = 0;
	luat_ctiot_get_ep(userEp);
	new_ep = lua_tolstring(L, 1, &len);
	if (len < 40 && len)
	{
		memset(userEp, 0, 40);
		memcpy(userEp, new_ep, len);
		new_set = 1;
	}
	if (new_set)
	{
		luat_ctiot_set_ep(userEp);
		luat_ctiot_get_ep(userEp);
	}
	lua_pushstring(L, userEp);
	return 1;
}

/**
 * @brief 设置和读取ctiot相关模式，有模式输入则设置，无论是否有模式输入，均输出当前模式
 * 
 * @param L 
 * @return int 
 */
static int l_ctiot_mode(lua_State *L)
{
	return 0;
}

/**
 * @brief 连接CTIOT，必须在设置完参数和模式后再使用
 * 
 * @param L 
 * @return int 
 */
static int l_ctiot_connect(lua_State *L)
{
	if (luat_ctiot_reg())
	{
		lua_pushboolean(L, 0);
	}
	else
	{
		lua_pushboolean(L, 1);
	}
	return 1;
}

/**
 * @brief 断开ctiot
 * 
 * @param L 
 * @return int 
 */
static int l_ctiot_disconnect(lua_State *L)
{
	luat_ctiot_dereg();
	return 0;
}

/**
 * @brief 发送数据给ctiot
 *
 * @param L
 * @return int
 */
static int l_ctiot_write(lua_State *L)
{
	const char *data;
	uint8_t mode;
	uint8_t seq;
	size_t len;
	data = lua_tolstring(L, 1, &len);
	if (!len)
	{
		lua_pushboolean(L, 0);
		lua_pushstring(L, "no data");
		return 2;
	}
	if (lua_isinteger(L, 2))
	{
		mode = lua_tointeger(L, 2);
	}
	if (lua_isinteger(L, 3))
	{
		seq = lua_tointeger(L, 3);
	}
	if (mode >= 4)
	{
		lua_pushboolean(L, 0);
		lua_pushstring(L, "mode error");
		return 2;
	}
	uint16_t result = luat_ctiot_send(data, len, mode, seq);
	if (result)
	{
		lua_pushboolean(L, 0);
		lua_pushfstring(L, "error code %d", result);
	}
	else
	{
		lua_pushboolean(L, 1);
		lua_pushnil(L);
	}
	return 2;
}

/**
 * @brief 读取已经接收到的数据
 * 
 * @param L 
 * @return int 
 */
static int l_ctiot_read(lua_State *L)
{
	return 0;
}

/**
 * @brief 发送更新注册信息给ctiot
 * 
 * @param L 
 * @return int 
 */
static int l_ctiot_update(lua_State *L)
{
	return 0;
}
#endif


#include "rotable.h"
static const rotable_Reg reg_ctiot[] =
{
#ifdef AIR302
    { "init", l_ctiot_init, 0},
    { "param", l_ctiot_param, 0},
	{ "ep", l_ctiot_ep, 0},
//	{ "mode", l_ctiot_mode, 0},
	{ "connect", l_ctiot_connect, 0},
	{ "disconnect", l_ctiot_disconnect, 0},
	{ "write", l_ctiot_write, 0},
//	{ "read", l_ctiot_read, 0},
//	{ "update", l_ctiot_update, 0},
    // ----- 类型常量
	{ "CON", NULL, 0},
	{ "NON", NULL, 1},
	{ "NON_REL", NULL, 2},
	{ "CON_REL", NULL, 3},
#endif
	{ NULL, NULL , 0}
};

LUAMOD_API int luaopen_ctiot( lua_State *L ) {
    rotable_newlib(L, reg_ctiot);
    return 1;
}
