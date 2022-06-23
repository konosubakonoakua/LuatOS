

#include "luat_base.h"
#include "luat_rtos.h"
#include "luat_gpio.h"
#include "luat_spi.h"
#include "luat_malloc.h"

#include "sx126x/radio.h"
#include "sx126x/sx126x.h"
#include "sx126x/sx126x-board.h"

#define LUAT_LOG_TAG "lora"
#include "luat_log.h"

extern uint8_t SX126xSpi;
extern uint8_t SX126xCsPin;
extern uint8_t SX126xResetPin;
extern uint8_t SX126xBusyPin;
extern uint8_t SX126xDio1Pin;
static RadioEvents_t RadioEvents;

enum{
LORA_TX_DONE,
LORA_RX_DONE,
LORA_TX_TIMEOUT,
LORA_RX_TIMEOUT,
LORA_RX_ERROR,
};

static int l_lora_handler(lua_State* L, void* ptr) {
    rtos_msg_t* msg = (rtos_msg_t*)lua_topointer(L, -1);
    int event = msg->arg1;
    lua_getglobal(L, "sys_pub");
    if (lua_isnil(L, -1)) {
        lua_pushinteger(L, 0);
        return 1;
    }
    switch (event){
    case LORA_TX_DONE: 
        lua_pushstring(L, "LORA_TX_DONE");
        lua_call(L, 1, 0);
        break;
    case LORA_RX_DONE: 
        lua_pushstring(L, "LORA_RX_DONE");
        lua_pushlstring(L, (const char *)msg->ptr,msg->arg2);
        lua_pushinteger(L, msg->arg2);
        lua_call(L, 3, 0);
        luat_heap_free(msg->ptr);
        break;
    case LORA_TX_TIMEOUT: 
        lua_pushstring(L, "LORA_TX_TIMEOUT");
        lua_call(L, 1, 0);
        break;
    case LORA_RX_TIMEOUT: 
        lua_pushstring(L, "LORA_RX_TIMEOUT");
        lua_call(L, 1, 0);
        break;
    case LORA_RX_ERROR: 
        lua_pushstring(L, "LORA_RX_ERROR");
        lua_call(L, 1, 0);
        break;
    }
}

void OnTxDone( void ){   
    rtos_msg_t msg;
    msg.handler = l_lora_handler;
    msg.ptr = NULL;
    msg.arg1 = LORA_TX_DONE;
    msg.arg2 = 0;
    luat_msgbus_put(&msg, 1);
}

void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr ){
    // printf("RxDone size:%d rssi:%d snr:%d\n",size,rssi,snr);
    // printf("RxDone payload: %.*s",size,payload);
    char* rx_buff = luat_heap_malloc(size);
    memcpy( rx_buff, payload, size );

    rtos_msg_t msg;
    msg.handler = l_lora_handler;
    msg.ptr = rx_buff;
    msg.arg1 = LORA_RX_DONE;
    msg.arg2 = size;
    luat_msgbus_put(&msg, 1);
}

void OnTxTimeout( void ){
    rtos_msg_t msg;
    msg.handler = l_lora_handler;
    msg.ptr = NULL;
    msg.arg1 = LORA_TX_TIMEOUT;
    msg.arg2 = 0;
    luat_msgbus_put(&msg, 1);
}

void OnRxTimeout( void ){
    rtos_msg_t msg;
    msg.handler = l_lora_handler;
    msg.ptr = NULL;
    msg.arg1 = LORA_RX_TIMEOUT;
    msg.arg2 = 0;
    luat_msgbus_put(&msg, 1);
}

void OnRxError( void ){
    rtos_msg_t msg;
    msg.handler = l_lora_handler;
    msg.ptr = NULL;
    msg.arg1 = LORA_RX_ERROR;
    msg.arg2 = 0;
    luat_msgbus_put(&msg, 1);
}

static int luat_lora_init(lua_State *L){
    size_t len = 0;
    const char* lora_ic = luaL_checklstring(L, 1, &len);
    if(strcmp("llcc68",lora_ic)== 0||strcmp("LLCC68",lora_ic)== 0){
        uint8_t mode,bandwidth,datarate,coderate,preambleLen,freqHopOn,hopPeriod,power,fdev,bandwidthAfc,payloadLen;
        uint32_t frequency,timeout,symbTimeout;
        bool fixLen,crcOn,iqInverted,rxContinuous;
        uint8_t id,cs,res,busy,dio1;

        if (lua_istable(L, 2)) {
            lua_pushstring(L, "mode");
            if (LUA_TNUMBER == lua_gettable(L, 2)) {
                mode = luaL_optinteger(L, -1,1);
            }
            lua_pop(L, 1);
            lua_pushstring(L, "bandwidth");
            if (LUA_TNUMBER == lua_gettable(L, 2)) {
                bandwidth = luaL_optinteger(L, -1,0);
            }
            lua_pop(L, 1);
            lua_pushstring(L, "datarate");
            if (LUA_TNUMBER == lua_gettable(L, 2)) {
                datarate = luaL_optinteger(L, -1,9);
            }
            lua_pop(L, 1);
            lua_pushstring(L, "coderate");
            if (LUA_TNUMBER == lua_gettable(L, 2)) {
                coderate = luaL_optinteger(L, -1,4);
            }
            lua_pop(L, 1);
            lua_pushstring(L, "preambleLen");
            if (LUA_TNUMBER == lua_gettable(L, 2)) {
                preambleLen = luaL_optinteger(L, -1,8);
            }
            lua_pop(L, 1);
            lua_pushstring(L, "fixLen");
            if (LUA_TBOOLEAN == lua_gettable(L, 2)) {
                fixLen = lua_toboolean(L, -1);
            }
            lua_pop(L, 1);
            lua_pushstring(L, "crcOn");
            if (LUA_TBOOLEAN == lua_gettable(L, 2)) {
                crcOn = lua_toboolean(L, -1);
            }
            lua_pop(L, 1);
            lua_pushstring(L, "freqHopOn");
            if (LUA_TNUMBER == lua_gettable(L, 2)) {
                freqHopOn = luaL_optinteger(L, -1,0);
            }
            lua_pop(L, 1);
            lua_pushstring(L, "hopPeriod");
            if (LUA_TNUMBER == lua_gettable(L, 2)) {
                hopPeriod = luaL_optinteger(L, -1,0);
            }
            lua_pop(L, 1);
            lua_pushstring(L, "iqInverted");
            if (LUA_TBOOLEAN == lua_gettable(L, 2)) {
                iqInverted = lua_toboolean(L, -1);
            }
            lua_pop(L, 1);
            lua_pushstring(L, "frequency");
            if (LUA_TNUMBER == lua_gettable(L, 2)) {
                frequency = luaL_optinteger(L, -1,433000000);
            }
            lua_pop(L, 1);
            lua_pushstring(L, "power");
            if (LUA_TNUMBER == lua_gettable(L, 2)) {
                power = luaL_optinteger(L, -1,22);
            }
            lua_pop(L, 1);
            lua_pushstring(L, "fdev");
            if (LUA_TNUMBER == lua_gettable(L, 2)) {
                fdev = luaL_optinteger(L, -1,0);
            }
            lua_pop(L, 1);
            lua_pushstring(L, "timeout");
            if (LUA_TNUMBER == lua_gettable(L, 2)) {
                timeout = luaL_optinteger(L, -1,3000);
            }
            lua_pop(L, 1);
            lua_pushstring(L, "bandwidthAfc");
            if (LUA_TNUMBER == lua_gettable(L, 2)) {
                bandwidthAfc = luaL_optinteger(L, -1,0);
            }
            lua_pop(L, 1);
            lua_pushstring(L, "symbTimeout");
            if (LUA_TNUMBER == lua_gettable(L, 2)) {
                symbTimeout = luaL_optinteger(L, -1,0);
            }
            lua_pop(L, 1);
            lua_pushstring(L, "payloadLen");
            if (LUA_TNUMBER == lua_gettable(L, 2)) {
                payloadLen = luaL_optinteger(L, -1,0);
            }
            lua_pop(L, 1);
            lua_pushstring(L, "rxContinuous");
            if (LUA_TBOOLEAN == lua_gettable(L, 2)) {
                rxContinuous = lua_toboolean(L, -1);
            }
            lua_pop(L, 1);
        }
        if (lua_istable(L, 3)) {
            lua_pushstring(L, "id");
            if (LUA_TNUMBER == lua_gettable(L, 3)) {
                id = luaL_checkinteger(L, -1);
            }
            lua_pop(L, 1);
            lua_pushstring(L, "cs");
            if (LUA_TNUMBER == lua_gettable(L, 3)) {
                cs = luaL_checkinteger(L, -1);
            }
            lua_pop(L, 1);
            lua_pushstring(L, "res");
            if (LUA_TNUMBER == lua_gettable(L, 3)) {
                res = luaL_checkinteger(L, -1);
            }
            lua_pop(L, 1);
            lua_pushstring(L, "busy");
            if (LUA_TNUMBER == lua_gettable(L, 3)) {
                busy = luaL_checkinteger(L, -1);
            }
            lua_pop(L, 1);
            lua_pushstring(L, "dio1");
            if (LUA_TNUMBER == lua_gettable(L, 3)) {
                dio1 = luaL_checkinteger(L, -1);
            }
            lua_pop(L, 1);
        }
        luat_spi_t sx126x_spi = {0};
        sx126x_spi.id = id;
        sx126x_spi.CPHA = 0;
        sx126x_spi.CPOL = 0;
        sx126x_spi.dataw = 8;
        sx126x_spi.bit_dict = 1;
        sx126x_spi.master = 1;
        sx126x_spi.mode = 0;
        sx126x_spi.bandrate = 20000000;
        sx126x_spi.cs = Luat_GPIO_MAX_ID;
        luat_spi_setup(&sx126x_spi);

        SX126xCsPin = cs;
        SX126xResetPin = res;
        SX126xBusyPin = busy;
        SX126xDio1Pin = dio1;

        luat_gpio_mode(SX126xCsPin, Luat_GPIO_OUTPUT, Luat_GPIO_PULLUP, Luat_GPIO_HIGH);
        luat_gpio_mode(SX126xResetPin, Luat_GPIO_OUTPUT, Luat_GPIO_PULLUP, Luat_GPIO_HIGH); 
        luat_gpio_mode(SX126xBusyPin, Luat_GPIO_INPUT, Luat_GPIO_PULLUP, Luat_GPIO_LOW); 
        luat_gpio_mode(SX126xDio1Pin, Luat_GPIO_INPUT, Luat_GPIO_PULLUP, Luat_GPIO_LOW); 

        RadioEvents.TxDone = OnTxDone;
        RadioEvents.RxDone = OnRxDone;
        RadioEvents.TxTimeout = OnTxTimeout;
        RadioEvents.RxTimeout = OnRxTimeout;
        RadioEvents.RxError = OnRxError;
        
        Radio.Init( &RadioEvents );
        Radio.SetChannel( frequency );

        Radio.SetTxConfig( mode, power, fdev, bandwidth,
                            datarate, coderate,
                            preambleLen, fixLen,
                            crcOn, freqHopOn, hopPeriod, iqInverted, timeout );
        
        Radio.SetRxConfig( mode, bandwidth, datarate,
                            coderate, bandwidthAfc, preambleLen,
                            symbTimeout, fixLen,
                            payloadLen, crcOn, freqHopOn, hopPeriod, iqInverted, rxContinuous );

        luat_start_rtos_timer(luat_create_rtos_timer(Radio.IrqProcess, NULL, NULL), 10, 1);
        return 1;
    }
}

static int luat_lora_send(lua_State *L){
    size_t len;
    const char* send_buff = luaL_checklstring(L, 1, &len);
    Radio.Standby();
    Radio.Send( send_buff, len);
}

static int luat_lora_recive(lua_State *L){
    int rx_timeout = luaL_optinteger(L, 1, 1000);
    Radio.Standby();
    Radio.Rx(rx_timeout); 
}

#include "rotable2.h"
static const rotable_Reg_t reg_lora[] =
{
    { "init",        ROREG_FUNC(luat_lora_init)},
    { "send",        ROREG_FUNC(luat_lora_send)},
    { "recive",      ROREG_FUNC(luat_lora_recive)},
	{ NULL,     {}}
};

LUAMOD_API int luaopen_lora( lua_State *L ) {
    luat_newlib2(L, reg_lora);
    return 1;
}
