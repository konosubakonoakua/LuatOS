/*
@module  mqtt
@summary mqtt客户端
@version 1.0
@date    2022.08.25
@demo socket
@tag LUAT_USE_MQTT
*/

#include "luat_base.h"

#include "luat_network_adapter.h"
#include "libemqtt.h"
#include "luat_rtos.h"
#include "luat_zbuff.h"
#include "luat_malloc.h"
#include "luat_mqtt.h"

#define LUAT_LOG_TAG "mqtt"
#include "luat_log.h"

#define LUAT_MQTT_CTRL_TYPE "MQTTCTRL*"

static luat_mqtt_ctrl_t * get_mqtt_ctrl(lua_State *L){
	if (luaL_testudata(L, 1, LUAT_MQTT_CTRL_TYPE)){
		return ((luat_mqtt_ctrl_t *)luaL_checkudata(L, 1, LUAT_MQTT_CTRL_TYPE));
	}else{
		return ((luat_mqtt_ctrl_t *)lua_touserdata(L, 1));
	}
}

static int32_t l_mqtt_callback(lua_State *L, void* ptr){
    rtos_msg_t* msg = (rtos_msg_t*)lua_topointer(L, -1);
    luat_mqtt_ctrl_t *mqtt_ctrl =(luat_mqtt_ctrl_t *)msg->ptr;
    switch (msg->arg1) {
		case MQTT_MSG_TIMER_PING : {
			luat_mqtt_ping(mqtt_ctrl);
			break;
		}
		case MQTT_MSG_RECONNECT : {
			luat_mqtt_reconnect(mqtt_ctrl);
			break;
		}
		case MQTT_MSG_PUBLISH : {
			luat_mqtt_msg_t *mqtt_msg =(luat_mqtt_msg_t *)msg->arg2;
			if (mqtt_ctrl->mqtt_cb) {
				luat_mqtt_msg_t *mqtt_msg =(luat_mqtt_msg_t *)msg->arg2;
				lua_geti(L, LUA_REGISTRYINDEX, mqtt_ctrl->mqtt_cb);
				if (lua_isfunction(L, -1)) {
					lua_geti(L, LUA_REGISTRYINDEX, mqtt_ctrl->mqtt_ref);
					lua_pushstring(L, "recv");
					lua_pushlstring(L, mqtt_msg->data,mqtt_msg->topic_len);
					lua_pushlstring(L, mqtt_msg->data+mqtt_msg->topic_len,mqtt_msg->payload_len);
					
					// 增加一个返回值meta，类型为table，包含qos、retain和dup
					// 	mqttc:on(function(mqtt_client, event, data, payload, meta)
            		// 		if event == "recv" then
            		//     	log.info("mqtt recv", "topic", data)
            		//     	log.info("mqtt recv", 'payload', payload)
            		//     	log.info("mqtt recv", 'meta.qos', meta.qos)
            		//     	log.info("mqtt recv", 'meta.retain', meta.retain)
            		//     	log.info("mqtt recv", 'meta.dup', meta.dup)
					lua_createtable(L, 0, 3);

					lua_pushliteral(L, "qos"); 
					lua_pushinteger(L, MQTTParseMessageQos(mqtt_ctrl->mqtt_packet_buffer));
					lua_settable(L, -3);

					lua_pushliteral(L, "retain"); 
					lua_pushinteger(L, MQTTParseMessageRetain(mqtt_ctrl->mqtt_packet_buffer));
					lua_settable(L, -3);

					lua_pushliteral(L, "dup"); 
					lua_pushinteger(L, MQTTParseMessageDuplicate(mqtt_ctrl->mqtt_packet_buffer));
					lua_settable(L, -3);

					// lua_call(L, 4, 0);
					lua_call(L, 5, 0);
				}
            }
			luat_heap_free(mqtt_msg);
            break;
        }
        case MQTT_MSG_CONNACK: {
			if (mqtt_ctrl->mqtt_cb) {
				lua_geti(L, LUA_REGISTRYINDEX, mqtt_ctrl->mqtt_cb);
				if (lua_isfunction(L, -1)) {
					lua_geti(L, LUA_REGISTRYINDEX, mqtt_ctrl->mqtt_ref);
					lua_pushstring(L, "conack");
					lua_call(L, 2, 0);
				}
				lua_getglobal(L, "sys_pub");
				if (lua_isfunction(L, -1)) {
					lua_pushstring(L, "MQTT_CONNACK");
					lua_geti(L, LUA_REGISTRYINDEX, mqtt_ctrl->mqtt_ref);
					lua_call(L, 2, 0);
				}
            }
            break;
        }
		case MQTT_MSG_PUBACK:
		case MQTT_MSG_PUBCOMP: {
			if (mqtt_ctrl->mqtt_cb) {
				lua_geti(L, LUA_REGISTRYINDEX, mqtt_ctrl->mqtt_cb);
				if (lua_isfunction(L, -1)) {
					lua_geti(L, LUA_REGISTRYINDEX, mqtt_ctrl->mqtt_ref);
					lua_pushstring(L, "sent");
					lua_pushinteger(L, msg->arg2);
					lua_call(L, 3, 0);
				}
            }
            break;
        }
		case MQTT_MSG_RELEASE: {
			if (mqtt_ctrl->mqtt_ref) {
				luaL_unref(L, LUA_REGISTRYINDEX, mqtt_ctrl->mqtt_ref);
				mqtt_ctrl->mqtt_ref = 0;
            }
            break;
        }
		default : {
			LLOGD("l_mqtt_callback error arg1:%d",msg->arg1);
            break;
        }
    }
    // lua_pushinteger(L, 0);
    return 0;
}

int l_luat_mqtt_msg_cb(luat_mqtt_ctrl_t * ptr, int arg1, int arg2) {
	rtos_msg_t msg = {
		.handler = l_mqtt_callback,
		.ptr = ptr,
		.arg1 = arg1,
		.arg2 = arg2
	};
	luat_msgbus_put(&msg, 0);
	return 0;
}

/*
订阅主题
@api mqttc:subscribe(topic, qos)
@string/table 主题
@int topic为string时生效 0/1/2 默认0
@return int 消息id,当qos为1时有效, 若底层返回失败,会返回nil
@usage 
-- 订阅单个topic, 且qos=0
mqttc:subscribe("/luatos/123456", 0)
-- 订阅单个topic, 且qos=1
mqttc:subscribe("/luatos/12345678", 1)
-- 订阅多个topic, 且使用不同的qos
mqttc:subscribe({["/luatos/1234567"]=1,["/luatos/12345678"]=2})
*/
static int l_mqtt_subscribe(lua_State *L) {
	size_t len = 0;
	int ret = 0;
	uint16_t msgid = 0;
	luat_mqtt_ctrl_t * mqtt_ctrl = (luat_mqtt_ctrl_t *)lua_touserdata(L, 1);
	if (lua_isstring(L, 2)){
		const char * topic = luaL_checklstring(L, 2, &len);
		uint8_t qos = luaL_optinteger(L, 3, 0);
		ret = mqtt_subscribe(&(mqtt_ctrl->broker), topic, &msgid, qos);
	}else if(lua_istable(L, 2)){
		lua_pushnil(L);
		while (lua_next(L, 2) != 0) {
			ret &= mqtt_subscribe(&(mqtt_ctrl->broker), lua_tostring(L, -2), &msgid, luaL_optinteger(L, -1, 0)) == 1 ? 1 : 0;
			lua_pop(L, 1);
		}
	}
	if (ret == 1) {
		lua_pushinteger(L, msgid);
		return 1;
	}
	else {
		return 0;
	}
}

/*
取消订阅主题
@api mqttc:unsubscribe(topic)
@string/table 主题
@usage 
mqttc:unsubscribe("/luatos/123456")
mqttc:unsubscribe({"/luatos/1234567","/luatos/12345678"})
*/
static int l_mqtt_unsubscribe(lua_State *L) {
	size_t len = 0;
	int ret = 0;
	uint16_t msgid = 0;
	luat_mqtt_ctrl_t * mqtt_ctrl = (luat_mqtt_ctrl_t *)lua_touserdata(L, 1);
	if (lua_isstring(L, 2)){
		const char * topic = luaL_checklstring(L, 2, &len);
		ret = mqtt_unsubscribe(&(mqtt_ctrl->broker), topic, &msgid);
	}else if(lua_istable(L, 2)){
		size_t count = lua_rawlen(L, 2);
		for (size_t i = 1; i <= count; i++){
			lua_geti(L, 2, i);
			const char * topic = luaL_checklstring(L, -1, &len);
			ret &= mqtt_unsubscribe(&(mqtt_ctrl->broker), topic, &msgid) == 1 ? 1 : 0;
			lua_pop(L, 1);
		}
	}
	if (ret == 1) {
		lua_pushinteger(L, msgid);
		return 1;
	}
	return 0;
}

/*
配置是否打开debug信息
@api mqttc:debug(onoff)
@boolean 是否打开debug开关
@return nil 无返回值
@usage mqttc:debug(true)
*/
static int l_mqtt_set_debug(lua_State *L){
	luat_mqtt_ctrl_t * mqtt_ctrl = get_mqtt_ctrl(L);
	if (lua_isboolean(L, 2)){
		mqtt_ctrl->netc->is_debug = lua_toboolean(L, 2);
	}
	return 0;
}

/*
mqtt客户端创建
@api mqtt.create(adapter,host,port,isssl,ca_file)
@int 适配器序号, 只能是network.ETH0,network.STA,network.AP,如果不填,会选择最后一个注册的适配器
@string 服务器地址,可以是域名, 也可以是ip
@int  	端口号
@bool/table  是否为ssl加密连接,默认不加密,true为无证书最简单的加密，table为有证书的加密 <br>server_cert 服务器ca证书数据 <br>client_cert 客户端ca证书数据 <br>client_key 客户端私钥加密数据 <br>client_password 客户端私钥口令数据
@return userdata 若成功会返回mqtt客户端实例,否则返回nil
@usage
-- 普通TCP链接
mqttc = mqtt.create(nil,"120.55.137.106", 1884)
-- 加密TCP链接
mqttc = mqtt.create(nil,"120.55.137.106", 8883, true)
-- 带证书的TCP链接
mqttc = mqtt.create(nil,"120.55.137.106", 8883, true, io.readFile("/luadb/ca.crt"), "123", "456")
*/
static int l_mqtt_create(lua_State *L) {
	int ret = 0;
	int adapter_index = luaL_optinteger(L, 1, network_get_last_register_adapter());
	if (adapter_index < 0 || adapter_index >= NW_ADAPTER_QTY){
		return 0;
	}
	luat_mqtt_ctrl_t *mqtt_ctrl = (luat_mqtt_ctrl_t *)lua_newuserdata(L, sizeof(luat_mqtt_ctrl_t));
	if (!mqtt_ctrl){
		LLOGE("out of memory when malloc mqtt_ctrl");
		return 0;
	}

	ret = luat_mqtt_init(mqtt_ctrl, adapter_index);
	if (ret) {
		LLOGE("mqtt init FAID ret %d", ret);
		return 0;
	}

	luat_mqtt_connopts_t opts = {0};

	// 连接参数相关
	const char *ip;
	size_t ip_len = 0;
#ifdef LUAT_USE_LWIP
	mqtt_ctrl->ip_addr.type = 0xff;
#else
	mqtt_ctrl->ip_addr.is_ipv6 = 0xff;
#endif
	if (lua_isinteger(L, 2)){
#ifdef LUAT_USE_LWIP
		mqtt_ctrl->ip_addr.type = IPADDR_TYPE_V4;
		mqtt_ctrl->ip_addr.u_addr.ip4.addr = lua_tointeger(L, 2);
#else
		mqtt_ctrl->ip_addr.is_ipv6 = 0;
		mqtt_ctrl->ip_addr.ipv4 = lua_tointeger(L, 2);
#endif
		ip = NULL;
		ip_len = 0;
	}else{
		ip_len = 0;
		opts.host = luaL_checklstring(L, 2, &ip_len);
		// TODO 判断 host的长度,超过191就不行了
	}

	opts.port = luaL_checkinteger(L, 3);

	// 加密相关
	if (lua_isboolean(L, 4)){
		opts.is_tls = lua_toboolean(L, 4);
	}

	if (lua_istable(L, 4)){
		opts.is_tls = 1;

		lua_pushstring(L, "server_cert");
		if (LUA_TSTRING == lua_gettable(L, 4)) {
			opts.server_cert = luaL_checklstring(L, -1, &opts.server_cert_len);
		}
		lua_pop(L, 1);

		lua_pushstring(L, "client_cert");
		if (LUA_TSTRING == lua_gettable(L, 4)) {
			opts.client_cert = luaL_checklstring(L, -1, &opts.client_cert_len);
		}
		lua_pop(L, 1);

		lua_pushstring(L, "client_key");
		if (LUA_TSTRING == lua_gettable(L, 4)) {
			opts.client_key = luaL_checklstring(L, -1, &opts.client_key_len);
		}
		lua_pop(L, 1);

		lua_pushstring(L, "client_password");
		if (LUA_TSTRING == lua_gettable(L, 4)) {
			opts.client_password = luaL_checklstring(L, -1, &opts.client_password_len);
		}
		lua_pop(L, 1);
	}
	
	ret = luat_mqtt_set_connopts(mqtt_ctrl, &opts);

	// TODO 判断ret, 如果初始化失败, 应该终止

	luaL_setmetatable(L, LUAT_MQTT_CTRL_TYPE);
	lua_pushvalue(L, -1);
	mqtt_ctrl->mqtt_ref = luaL_ref(L, LUA_REGISTRYINDEX);
	return 1;
}

/*
mqtt三元组配置
@api mqttc:auth(client_id,username,password)
@string 设备识别id,对于同一个mqtt服务器来说, 通常要求唯一,相同client_id会互相踢下线
@string 账号 可选
@string 密码 可选
@return nil 无返回值
@usage
mqttc:auth("123456789","username","password")
*/
static int l_mqtt_auth(lua_State *L) {
	luat_mqtt_ctrl_t * mqtt_ctrl = get_mqtt_ctrl(L);
	const char *client_id = luaL_optstring(L, 2, "");
	const char *username = luaL_optstring(L, 3, "");
	const char *password = luaL_optstring(L, 4, "");
	mqtt_init(&(mqtt_ctrl->broker), client_id);
	mqtt_init_auth(&(mqtt_ctrl->broker), username, password);
	return 0;
}

/*
mqtt心跳设置
@api mqttc:keepalive(time)
@int 可选 单位s 默认240s. 最先15,最高600
@return nil 无返回值
@usage 
mqttc:keepalive(30)
*/
static int l_mqtt_keepalive(lua_State *L) {
	luat_mqtt_ctrl_t * mqtt_ctrl = get_mqtt_ctrl(L);
	int timeout = luaL_optinteger(L, 2, 240);
	if (timeout < 15)
		timeout = 15;
	if (timeout > 600)
		timeout = 600;
	mqtt_ctrl->keepalive = timeout;
	return 0;
}

/*
注册mqtt回调
@api mqttc:on(cb)
@function cb mqtt回调,参数包括mqtt_client, event, data, payload
@return nil 无返回值
@usage 
mqttc:on(function(mqtt_client, event, data, payload)
	-- 用户自定义代码
	log.info("mqtt", "event", event, mqtt_client, data, payload)
end)
*/
static int l_mqtt_on(lua_State *L) {
	luat_mqtt_ctrl_t * mqtt_ctrl = get_mqtt_ctrl(L);
	if (mqtt_ctrl->mqtt_cb != 0) {
		luaL_unref(L, LUA_REGISTRYINDEX, mqtt_ctrl->mqtt_cb);
		mqtt_ctrl->mqtt_cb = 0;
	}
	if (lua_isfunction(L, 2)) {
		lua_pushvalue(L, 2);
		mqtt_ctrl->mqtt_cb = luaL_ref(L, LUA_REGISTRYINDEX);
	}
	return 0;
}

/*
连接服务器
@api mqttc:connect()
@return boolean 发起成功返回true, 否则返回false
@usage
-- 开始建立连接
mqttc:connect()
-- 本函数仅代表发起成功, 后续仍需根据ready函数判断mqtt是否连接正常
*/
static int l_mqtt_connect(lua_State *L) {
	luat_mqtt_ctrl_t * mqtt_ctrl = get_mqtt_ctrl(L);
	int ret = luat_mqtt_connect(mqtt_ctrl);
	if (ret) {
		LLOGE("socket connect ret=%d\n", ret);
		luat_mqtt_close_socket(mqtt_ctrl);
		lua_pushboolean(L, 0);
		return 1;
	}
	lua_pushboolean(L, 1);
	return 1;
}

/*
自动重连
@api mqttc:autoreconn(reconnect, reconnect_time)
@bool 是否自动重连
@int 自动重连周期 单位ms 默认3000ms
@usage 
mqttc:autoreconn(true)
*/
static int l_mqtt_autoreconn(lua_State *L) {
	luat_mqtt_ctrl_t * mqtt_ctrl = get_mqtt_ctrl(L);
	if (lua_isboolean(L, 2)){
		mqtt_ctrl->reconnect = lua_toboolean(L, 2);
	}
	mqtt_ctrl->reconnect_time = luaL_optinteger(L, 3, 3000);
	if (mqtt_ctrl->reconnect && mqtt_ctrl->reconnect_time < 1000)
		mqtt_ctrl->reconnect_time = 1000;
	return 0;
}

/*
发布消息
@api mqttc:publish(topic, data, qos, retain)
@string 主题,必填
@string 消息,必填,但长度可以是0
@int 消息级别 0/1 默认0
@int 是否存档, 0/1,默认0
@return int 消息id, 当qos为1或2时会有效值. 若底层返回是否, 会返回nil
@usage 
mqttc:publish("/luatos/123456", "123")
*/
static int l_mqtt_publish(lua_State *L) {
	uint32_t message_id ,payload_len= 0;
	luat_mqtt_ctrl_t * mqtt_ctrl = get_mqtt_ctrl(L);
	const char * topic = luaL_checkstring(L, 2);
	const char * payload = NULL;
	luat_zbuff_t *buff = NULL;
	if (lua_isstring(L, 3)){
		payload = luaL_checklstring(L, 3, &payload_len);
	}else if (luaL_testudata(L, 3, LUAT_ZBUFF_TYPE)){
		buff = ((luat_zbuff_t *)luaL_checkudata(L, 3, LUAT_ZBUFF_TYPE));
		payload = buff->addr;
		payload_len = buff->used;
	}else{
		LLOGD("only support string or zbuff");
	}
	// LLOGD("payload_len:%d",payload_len);
	uint8_t qos = luaL_optinteger(L, 4, 0);
	uint8_t retain = luaL_optinteger(L, 5, 0);
	int ret = mqtt_publish_with_qos(&(mqtt_ctrl->broker), topic, payload,payload_len, retain, qos, &message_id);
	if (ret != 1){
		return 0;
	}
	if (qos == 0){
		rtos_msg_t msg = {0};
    	msg.handler = l_mqtt_callback;
		msg.ptr = mqtt_ctrl;
		msg.arg1 = MQTT_MSG_PUBACK;
		msg.arg2 = message_id;
		luat_msgbus_put(&msg, 0);
	}
	lua_pushinteger(L, message_id);
	return 1;
}

/*
mqtt客户端关闭(关闭后资源释放无法再使用)
@api mqttc:close()
@usage 
mqttc:close()
*/
static int l_mqtt_close(lua_State *L) {
	luat_mqtt_ctrl_t * mqtt_ctrl = get_mqtt_ctrl(L);
	mqtt_disconnect(&(mqtt_ctrl->broker));
	luat_mqtt_close_socket(mqtt_ctrl);
	if (mqtt_ctrl->mqtt_cb != 0) {
		luaL_unref(L, LUA_REGISTRYINDEX, mqtt_ctrl->mqtt_cb);
		mqtt_ctrl->mqtt_cb = 0;
	}
	luat_mqtt_release_socket(mqtt_ctrl);
	return 0;
}

/*
mqtt客户端是否就绪
@api mqttc:ready()
@return bool 客户端是否就绪
@usage 
local error = mqttc:ready()
*/
static int l_mqtt_ready(lua_State *L) {
	luat_mqtt_ctrl_t * mqtt_ctrl = get_mqtt_ctrl(L);
	lua_pushboolean(L, mqtt_ctrl->mqtt_state > 0 ? 1 : 0);
	return 1;
}

static int _mqtt_struct_newindex(lua_State *L);

void luat_mqtt_struct_init(lua_State *L) {
    luaL_newmetatable(L, LUAT_MQTT_CTRL_TYPE);
    lua_pushcfunction(L, _mqtt_struct_newindex);
    lua_setfield( L, -2, "__index" );
    lua_pop(L, 1);
}

#include "rotable2.h"
static const rotable_Reg_t reg_mqtt[] =
{
	{"create",			ROREG_FUNC(l_mqtt_create)},
	{"auth",			ROREG_FUNC(l_mqtt_auth)},
	{"keepalive",		ROREG_FUNC(l_mqtt_keepalive)},
	{"on",				ROREG_FUNC(l_mqtt_on)},
	{"connect",			ROREG_FUNC(l_mqtt_connect)},
	{"autoreconn",		ROREG_FUNC(l_mqtt_autoreconn)},
	{"publish",			ROREG_FUNC(l_mqtt_publish)},
	{"subscribe",		ROREG_FUNC(l_mqtt_subscribe)},
	{"unsubscribe",		ROREG_FUNC(l_mqtt_unsubscribe)},
	{"close",			ROREG_FUNC(l_mqtt_close)},
	{"ready",			ROREG_FUNC(l_mqtt_ready)},

	{ NULL,             ROREG_INT(0)}
};

static int _mqtt_struct_newindex(lua_State *L) {
	rotable_Reg_t* reg = reg_mqtt;
    const char* key = luaL_checkstring(L, 2);
	while (1) {
		if (reg->name == NULL)
			return 0;
		if (!strcmp(reg->name, key)) {
			lua_pushcfunction(L, reg->value.value.func);
			return 1;
		}
		reg ++;
	}
    //return 0;
}
static const rotable_Reg_t reg_mqtt_emtry[] =
{
	{ NULL,             ROREG_INT(0)}
};

LUAMOD_API int luaopen_mqtt( lua_State *L ) {

#ifdef LUAT_USE_NETWORK
    luat_newlib2(L, reg_mqtt);
	luat_mqtt_struct_init(L);
    return 1;
#else
	luat_newlib2(L, reg_mqtt_emtry);
    return 1;
	LLOGE("mqtt require network enable!!");
#endif
}
