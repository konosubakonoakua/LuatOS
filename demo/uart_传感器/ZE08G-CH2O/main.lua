-- UART1支持600 1200 2400 4800 9600波特率下休眠仍然接收数据，并且不丢失数据
-- 其他波特率时，在休眠后通过UART1的RX唤醒，注意唤醒开始所有连续数据会丢失，所以要发2次，第一次发送字节后，会有提示，然后再发送数据
-- LuaTools需要PROJECT和VERSION这两个信息
PROJECT = "ZE08G-CH2O"
VERSION = "1.0.0"

log.info("main", PROJECT, VERSION)

-- 引入必要的库文件(lua编写), 内部库不需要require
sys = require("sys")
local ch2o = require "ze08g_ch2o"

local uartid = 1 -- 根据实际设备选取不同的uartid

-- 下面是模拟uart的配置
-- local tx_pin = 11       -- tx的pin脚
-- local rx_pin = 9        -- rx的pin脚
-- local uartid = uart.createSoft(tx_pin,0,rx_pin,2)

sys.taskInit(function ()
    local result = ch2o.init(uartid)
    if not result then return end
    
    log.info("result:", result)
    while true do
        sys.wait(1000)
        log.info("气体浓度值 PPB：", ch2o.getPPB())
        log.info("百万分比浓度 PPM：", ch2o.getPPM())
    end
end)

-- 用户代码已结束---------------------------------------------
-- 结尾总是这一句
sys.run()
-- sys.run()之后后面不要加任何语句!!!!!
