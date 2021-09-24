-- LuaTools需要PROJECT和VERSION这两个信息
PROJECT = "uart_irq"
VERSION = "1.0.0"

-- 引入必要的库文件(lua编写), 内部库不需要require
local sys = require "sys"

--添加硬狗防止程序卡死
wdt.init(15000)--初始化watchdog设置为15s
sys.timerLoopStart(wdt.feed, 10000)--10s喂一次狗

log.info("main", "uart demo")

local uartid = 1

--初始化
local result = uart.setup(
    uartid,--串口id
    115200,--波特率
    8,--数据位
    1--停止位
)


--循环发数据
sys.timerLoopStart(uart.write,1000,uartid,"test")
uart.on(uartid, "receive", function(id, len)
    log.info("uart", "receive", id, len, uart.read(uartid, len))
end)
uart.on(uartid, "sent", function(id)
    log.info("uart", "sent", id)
end)

sys.taskInit(function()
    while 1 do
        sys.wait(500)
    end
end)


-- 用户代码已结束---------------------------------------------
-- 结尾总是这一句
sys.run()
-- sys.run()之后后面不要加任何语句!!!!!
