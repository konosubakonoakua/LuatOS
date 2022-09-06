/*
@module  http2
@summary http2客户端
@version 1.0
@date    2022.09.05
@demo network
*/

#include "luat_base.h"

#ifdef LUAT_USE_NETWORK

#include "luat_network_adapter.h"
#include "luat_rtos.h"
#include "luat_msgbus.h"
#include "luat_fs.h"
#include "luat_malloc.h"

#define LUAT_LOG_TAG "http"
#include "luat_log.h"

#define HTTP_REQUEST_BUF_LEN_MAX 1024
typedef struct{
	network_ctrl_t *netc;		// http netc
	luat_ip_addr_t ip_addr;		// http ip
	uint8_t is_tls;
	const char *host; 			// http host
	uint16_t remote_port; 		// 远程端口号
	const char *url;
	const char *uri;
	const char *method;
	const char *header;
	const char *body;
	size_t body_len;
	const char *dst;
	uint8_t is_download;
	uint8_t request_message[HTTP_REQUEST_BUF_LEN_MAX];
	uint8_t *reply_message;
	uint32_t reply_message_len;
	uint64_t* idp;
	uint16_t timeout;
}luat_http_ctrl_t;

static int http_close(luat_http_ctrl_t *http_ctrl){
	if (http_ctrl->netc){
		network_force_close_socket(http_ctrl->netc);
		network_release_ctrl(http_ctrl->netc);
	}
	if (http_ctrl->host){
		luat_heap_free(http_ctrl->host);
	}
	if (http_ctrl->url){
		luat_heap_free(http_ctrl->url);
	}
	if (http_ctrl->uri){
		luat_heap_free(http_ctrl->uri);
	}
	if (http_ctrl->method){
		luat_heap_free(http_ctrl->method);
	}
	if (http_ctrl->header){
		luat_heap_free(http_ctrl->header);
	}
	if (http_ctrl->body){
		luat_heap_free(http_ctrl->body);
	}
	if (http_ctrl->dst){
		luat_heap_free(http_ctrl->dst);
	}
	if (http_ctrl->reply_message){
		luat_heap_free(http_ctrl->reply_message);
	}
	if (http_ctrl->idp){
		luat_heap_free(http_ctrl->idp);
	}
	luat_heap_free(http_ctrl);
	return 0;
}

static int http_body_len(char *headers){
	char *length_str = strstr(headers,"Content-Length")+15;
	char *Content_Length_end = strstr(length_str,"\r\n");
	char len[20] = {0};
	strncpy(len, length_str,Content_Length_end-length_str);
	return atoi(len);
}

static int32_t l_http_callback(lua_State *L, void* ptr){
	char code[6] = {0};
    rtos_msg_t* msg = (rtos_msg_t*)lua_topointer(L, -1);
    luat_http_ctrl_t *http_ctrl =(luat_http_ctrl_t *)msg->ptr;
    uint64_t* idp = (uint64_t*)http_ctrl->idp;
	uint16_t code_offset = strlen("HTTP/1.x ");
	uint16_t code_len = 3;
	char *header = strstr(http_ctrl->reply_message,"\r\n")+2;
	uint16_t content_len = http_body_len(header);
	char *body_rec = strstr(header,"\r\n\r\n")+4;
	uint16_t body_offset = strlen(body_rec);
	uint16_t header_len = strlen(header)-strlen(body_rec)-4;
	strncpy(code, http_ctrl->reply_message+code_offset,code_len);
	lua_pushinteger(L, atoi(code));
	lua_pushlstring(L, header, header_len); // TODO 需要解成table,或者当前设置为空table也行
	if (http_ctrl->is_download){
		luat_fs_remove(http_ctrl->dst);
		FILE *fd_out = luat_fs_fopen(http_ctrl->dst, "w+");
		if (!fd_out) {
			LLOGE("create file fail! %s", http_ctrl->dst);
		}
		else {
			luat_fs_fwrite(body_rec, 1, body_offset, fd_out);
			luat_fs_fclose(fd_out);
		}
		luat_cbcwait(L, *idp, 2);
	}else{
		lua_pushlstring(L, body_rec, body_offset);
		luat_cbcwait(L, *idp, 3);
	}
	return 0;
}

static int http_read_packet(luat_http_ctrl_t *http_ctrl){
	if (!strncmp("HTTP/1.", http_ctrl->reply_message, strlen("HTTP/1."))){
		uint16_t code_offset = strlen("HTTP/1.x ");
		uint16_t code_len = 3;
		char *header = strstr(http_ctrl->reply_message,"\r\n")+2;
		uint16_t content_len = http_body_len(header);
		char *body_rec = strstr(header,"\r\n\r\n")+4; // TODO 如果没找到, 还需要等下一波数据
		uint16_t body_offset = strlen(body_rec); // TODO 不能通过strlen判断, 要根据reply_message_len
		// LLOGD("l_http_callback content_len:%d body_offset:%d",content_len,body_offset);
		if (content_len == body_offset){
			rtos_msg_t msg = {0};
    		msg.handler = l_http_callback;
			msg.ptr = http_ctrl;
			luat_msgbus_put(&msg, 0);
		}
	}else{
		// TODO 如果没有数据, 那wait就不会返回了, 需要发消息
		http_close(http_ctrl);
	}
	return 0;
}

static uint32_t http_send(luat_http_ctrl_t *http_ctrl, uint8_t* data, size_t len) {
	if (len == 0)
		return 0;
	uint32_t tx_len = 0;
	network_tx(http_ctrl->netc, data, len, 0, http_ctrl->ip_addr.is_ipv6?NULL:&(http_ctrl->ip_addr), NULL, &tx_len, 0);
	return tx_len;
}

static int32_t luat_lib_http_callback(void *data, void *param){
	OS_EVENT *event = (OS_EVENT *)data;
	luat_http_ctrl_t *http_ctrl =(luat_http_ctrl_t *)param;
	int ret = 0;
	// LLOGD("LINK %d ON_LINE %d EVENT %d TX_OK %d CLOSED %d",EV_NW_RESULT_LINK & 0x0fffffff,EV_NW_RESULT_CONNECT & 0x0fffffff,EV_NW_RESULT_EVENT & 0x0fffffff,EV_NW_RESULT_TX & 0x0fffffff,EV_NW_RESULT_CLOSE & 0x0fffffff);
	// LLOGD("luat_lib_mqtt_callback %d %d",event->ID & 0x0fffffff,event->Param1);
	if (event->ID == EV_NW_RESULT_LINK){
		if(network_connect(http_ctrl->netc, http_ctrl->host, strlen(http_ctrl->host), http_ctrl->ip_addr.is_ipv6?NULL:&(http_ctrl->ip_addr), http_ctrl->remote_port, 0) < 0){
			network_close(http_ctrl->netc, 0);
			http_close(http_ctrl);
			return -1;
    	}
	}else if(event->ID == EV_NW_RESULT_CONNECT){
		//memset(http_ctrl->request_message, 0, HTTP_REQUEST_BUF_LEN_MAX);
		uint32_t tx_len = 0;
		// 发送请求行
		snprintf(http_ctrl->request_message, HTTP_REQUEST_BUF_LEN_MAX, "%s %s HTTP/1.0\r\n", http_ctrl->method, http_ctrl->uri);
		http_send(http_ctrl, http_ctrl->request_message, strlen(http_ctrl->request_message));
		// 强制添加host. TODO 判断自定义headers是否有host
		snprintf(http_ctrl->request_message, HTTP_REQUEST_BUF_LEN_MAX,  "Host: %s\r\n", http_ctrl->host);
		http_send(http_ctrl, http_ctrl->request_message, strlen(http_ctrl->request_message));
		// 发送自定义头部
		if (http_ctrl->header){
			http_send(http_ctrl, http_ctrl->header, strlen(http_ctrl->header));
		}
		// 结束头部
		http_send(http_ctrl, "\r\n", 2);
		// 发送body
		if (http_ctrl->body){
			http_send(http_ctrl, http_ctrl->body, http_ctrl->body_len);
		}
		//--------------------------------------------
		// 清理资源
		if (http_ctrl->host){
			luat_heap_free(http_ctrl->host);
			http_ctrl->host = NULL;
		}
		if (http_ctrl->url){
			luat_heap_free(http_ctrl->url);
			http_ctrl->url = NULL;
		}
		if (http_ctrl->uri){
			luat_heap_free(http_ctrl->uri);
			http_ctrl->uri = NULL;
		}
		if (http_ctrl->method){
			luat_heap_free(http_ctrl->method);
			http_ctrl->method = NULL;
		}
		if (http_ctrl->header){
			luat_heap_free(http_ctrl->header);
			http_ctrl->header = NULL;
		}
		if (http_ctrl->body){
			luat_heap_free(http_ctrl->body);
			http_ctrl->body = NULL;
			http_ctrl->body_len = 0;
		}
		//------------------------------
	}else if(event->ID == EV_NW_RESULT_EVENT){
		if (event->Param1==0){
			uint32_t total_len = 0;
			uint32_t rx_len = 0;
			int result = network_rx(http_ctrl->netc, NULL, 0, 0, NULL, NULL, &total_len);
			if (0 == http_ctrl->reply_message_len){
				http_ctrl->reply_message = luat_heap_malloc(total_len + 1);
				http_ctrl->reply_message[total_len] = 0x00;
			}else{
				http_ctrl->reply_message = luat_heap_realloc(http_ctrl->reply_message,http_ctrl->reply_message_len+total_len+1);
				http_ctrl->reply_message[http_ctrl->reply_message_len+total_len] = 0x00;
			}
			result = network_rx(http_ctrl->netc, http_ctrl->reply_message+(http_ctrl->reply_message_len), total_len, 0, NULL, NULL, &rx_len);
			if (rx_len == 0||result!=0) {
				http_close(http_ctrl);
				return -1;
			}
			http_ctrl->reply_message_len += total_len;
			http_read_packet(http_ctrl);
		}
	}else if(event->ID == EV_NW_RESULT_TX){

	}else if(event->ID == EV_NW_RESULT_CLOSE){

	}
	if (event->Param1){
		LLOGD("luat_lib_mqtt_callback http_ctrl close %d %d",event->ID & 0x0fffffff,event->Param1);
		http_close(http_ctrl);
		return -1;
	}
	network_wait_event(http_ctrl->netc, NULL, 0, NULL);
    return 0;
}

static int http_add_header(luat_http_ctrl_t *http_ctrl, const char* name, const char* value){
	// LLOGD("http_add_header name:%s value:%s",name,value);
	if (http_ctrl->header){
		http_ctrl->header = luat_heap_realloc(http_ctrl->header, strlen(http_ctrl->header)+strlen(name)+strlen(value)+1);
		strncat(http_ctrl->header, name, strlen(name));
		strncat(http_ctrl->header, ":", 1);
		strncat(http_ctrl->header, value, strlen(value));
		strncat(http_ctrl->header, "\r\n", 2);
	}else{
		http_ctrl->header = luat_heap_malloc(strlen(name)+strlen(value)+1);
		memset(http_ctrl->header, 0, strlen(name)+strlen(value)+1);
		sprintf(http_ctrl->header, "%s:%s\r\n", name,value);
	}
	// LLOGD("http_ctrl->header:%s",http_ctrl->header);
}

static int http_set_url(luat_http_ctrl_t *http_ctrl) {
	char *tmp = http_ctrl->url;
    if (!strncmp("https://", http_ctrl->url, strlen("https://"))) {
        http_ctrl->is_tls = 1;
        tmp += strlen("https://");
    }
    else if (!strncmp("http://", http_ctrl->url, strlen("http://"))) {
        http_ctrl->is_tls = 0;
        tmp += strlen("http://");
    }
    else {
        LLOGI("only http/https supported %s", http_ctrl->url);
        return -1;
    }

	int tmplen = strlen(tmp);
	if (tmplen < 5) {
        LLOGI("url too short %s", http_ctrl->url);
        return -1;
    }
	char tmphost[256] = {0};
    char *tmpuri = NULL;
    for (size_t i = 0; i < tmplen; i++){
        if (tmp[i] == '/') {
            if (i > 255) {
                LLOGI("host too long %s", http_ctrl->url);
                return -1;
            } 
            memcpy(tmphost, tmp, i);
            tmpuri = tmp + i;
            break;
        }
    }
	if (strlen(tmphost) < 1) {
        LLOGI("host not found %s", http_ctrl->url);
        return -1;
    }
    if (strlen(tmpuri) == 0) {
        tmpuri = "/";
    }
    for (size_t i = 1; i < strlen(tmphost); i++){
        if (tmp[i] == ":") {
            tmp[i] = 0x00;
            http_ctrl->remote_port = atoi(&tmp[i+1]);
            break;
        }
    }
    if (http_ctrl->remote_port <= 0) {
        if (http_ctrl->is_tls)
            http_ctrl->remote_port = 443;
        else
            http_ctrl->remote_port = 80;
    }
    // LLOGD("tmphost:%s",tmphost);
	// LLOGD("tmpuri:%s",tmpuri);
    http_ctrl->host = luat_heap_malloc(strlen(tmphost) + 1);
    if (http_ctrl->host == NULL) {
        LLOGE("out of memory when malloc host");
        return -1;
    }
    memcpy(http_ctrl->host, tmphost, strlen(tmphost) + 1);

    http_ctrl->uri = luat_heap_malloc(strlen(tmpuri) + 1);
    if (http_ctrl->uri == NULL) {
        LLOGE("out of memory when malloc url");
        return -1;
    }
    memcpy(http_ctrl->uri, tmpuri, strlen(tmpuri) + 1);

	// LLOGD("http_ctrl->uri:%s",http_ctrl->uri);
	// LLOGD("http_ctrl->host:%s",http_ctrl->host);
	// LLOGD("http_ctrl->port:%d",http_ctrl->remote_port);
	return 0;
}

/*
http2客户端
@api http2.request(method,url,headers,body,opts,ca_file)
@string 请求方法, 支持 GET/POST
@string url地址
@tabal  请求头 可选 例如{["Content-Type"] = "application/x-www-form-urlencoded"}
@string body 可选
@tabal  额外配置 可选 包含dst:下载路径,可选 adapter:选择使用网卡,可选
@string 证书 可选
@return int code
@return string headers 
@return string body
@usage 
local code, headers, body = http2.request("GET","http://site0.cn/api/httptest/simple/time").wait()
log.info("http2.get", code, headers, body)
*/
static int l_http_request(lua_State *L) {
	size_t client_cert_len, client_key_len, client_password_len,len;
	const char *client_cert = NULL;
	const char *client_key = NULL;
	const char *client_password = NULL;
	int adapter_index;
	char body_len[6] = {0}; 
	// mbedtls_debug_set_threshold(4);
	luat_http_ctrl_t *http_ctrl = (luat_http_ctrl_t *)luat_heap_malloc(sizeof(luat_http_ctrl_t));
	if (!http_ctrl){
		goto error;
	}
	memset(http_ctrl, 0, sizeof(luat_http_ctrl_t));

	if (lua_istable(L, 5)){
		lua_pushstring(L, "adapter");
		if (LUA_TNUMBER == lua_gettable(L, 5)) {
			adapter_index = luaL_optinteger(L, -1, network_get_last_register_adapter());
		}else{
			adapter_index = network_get_last_register_adapter();
		}
		lua_pop(L, 1);

		lua_pushstring(L, "timeout");
		if (LUA_TNUMBER == lua_gettable(L, 5)) {
			http_ctrl->timeout = luaL_optinteger(L, -1, 0);
		}
		lua_pop(L, 1);

		lua_pushstring(L, "dst");
		if (LUA_TSTRING == lua_gettable(L, 5)) {
			const char *dst = luaL_checklstring(L, -1, &len);
			http_ctrl->dst = luat_heap_malloc(len + 1);
			memset(http_ctrl->dst, 0, len + 1);
			memcpy(http_ctrl->dst, dst, len);
			http_ctrl->is_download = 1;
		}
		lua_pop(L, 1);

	}else{
		adapter_index = network_get_last_register_adapter();
	}

	if (adapter_index < 0 || adapter_index >= NW_ADAPTER_QTY){
		goto error;
	}

	http_ctrl->netc = network_alloc_ctrl(adapter_index);
	if (!http_ctrl->netc){
		LLOGE("create fail");
		goto error;
	}
	network_init_ctrl(http_ctrl->netc, NULL, luat_lib_http_callback, http_ctrl);

	http_ctrl->netc->is_debug = 1;
	network_set_base_mode(http_ctrl->netc, 1, 10000, 0, 0, 0, 0);
	network_set_local_port(http_ctrl->netc, 0);

	const char *method = luaL_optlstring(L, 1, "GET", &len);
	http_ctrl->method = luat_heap_malloc(len + 1);
	memset(http_ctrl->method, 0, len + 1);
	memcpy(http_ctrl->method, method, len);
	// LLOGD("method:%s",http_ctrl->method);

	const char *url = luaL_checklstring(L, 2, &len);
	http_ctrl->url = luat_heap_malloc(len + 1);
	memset(http_ctrl->url, 0, len + 1);
	memcpy(http_ctrl->url, url, len);

	// LLOGD("http_ctrl->url:%s",http_ctrl->url);

	if (lua_istable(L, 3)) {
		lua_pushnil(L);
		while (lua_next(L, 3) != 0) {
			const char *name = lua_tostring(L, -2);
			const char *value = lua_tostring(L, -1);
			http_add_header(http_ctrl,name,value);
			lua_pop(L, 1);
		}
	}
	if (lua_isstring(L, 4)) {
		const char *body = luaL_checklstring(L, 4, &(http_ctrl->body_len));
		http_ctrl->body = luat_heap_malloc((http_ctrl->body_len) + 1);
		memset(http_ctrl->body, 0, (http_ctrl->body_len) + 1);
		memcpy(http_ctrl->body, body, (http_ctrl->body_len));
		sprintf(body_len, "%d",(http_ctrl->body_len));
		http_add_header(http_ctrl,"Content-Length",body_len);
	}
	// else{
	// 	http_add_header(http_ctrl,"Content-Length","0");
	// }
	
	if (http_ctrl->is_tls){
		if (lua_isstring(L, 6)){
			client_cert = luaL_checklstring(L, 6, &client_cert_len);
		}
		if (lua_isstring(L, 7)){
			client_key = luaL_checklstring(L, 7, &client_key_len);
		}
		if (lua_isstring(L, 8)){
			client_password = luaL_checklstring(L, 8, &client_password_len);
		}
		network_init_tls(http_ctrl->netc, client_cert?2:0);
		if (client_cert){
			network_set_client_cert(http_ctrl->netc, client_cert, client_cert_len,
					client_key, client_key_len,
					client_password, client_password_len);
		}
	}else{
		network_deinit_tls(http_ctrl->netc);
	}
	int ret = http_set_url(http_ctrl);
	if (ret){
		goto error;
	}
	
	if (!strncmp("GET", http_ctrl->method, strlen("GET"))) {
        LLOGI("HTTP GET");
    }
    else if (!strncmp("POST", http_ctrl->method, strlen("POST"))) {
        LLOGI("HTTP POST");
    }else {
        LLOGI("only GET/POST supported %s", http_ctrl->method);
        goto error;
    }

	http_ctrl->ip_addr.is_ipv6 = 0xff;

	network_wait_link_up(http_ctrl->netc, 0);
	if (ret == 0){
		if(network_connect(http_ctrl->netc, http_ctrl->host, strlen(http_ctrl->host), http_ctrl->ip_addr.is_ipv6?NULL:&(http_ctrl->ip_addr), http_ctrl->remote_port, 0) < 0){
        	network_close(http_ctrl->netc, 0);
        	goto error;
    	}
	}

	uint64_t id = luat_pushcwait(L);
	http_ctrl->idp = (uint64_t*)luat_heap_malloc(sizeof(uint64_t));
    memcpy(http_ctrl->idp, &id, sizeof(uint64_t));
    return 1;
error:
	http_close(http_ctrl);
	return 0;
}

#include "rotable2.h"
static const rotable_Reg_t reg_http[] =
{
	{"request",			ROREG_FUNC(l_http_request)},
	{ NULL,             ROREG_INT(0)}
};

LUAMOD_API int luaopen_http( lua_State *L ) {
    luat_newlib2(L, reg_http);
    return 1;
}

#else

#define LUAT_LOG_TAG "http2"
#include "luat_log.h"

#include "rotable2.h"
static const rotable_Reg_t reg_http[] =
{
	{ NULL,             ROREG_INT(0)}
};
LUAMOD_API int luaopen_http( lua_State *L ) {
    luat_newlib2(L, reg_http);
	LLOGE("reg_http2 require network enable!!");
    return 1;
}
#endif


