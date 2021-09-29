
-- LuaTools需要PROJECT和VERSION这两个信息
PROJECT = "gpiodemo"
VERSION = "1.0.0"

-- sys库是标配
_G.sys = require("sys")

--添加硬狗防止程序卡死
wdt.init(15000)--初始化watchdog设置为15s
sys.timerLoopStart(wdt.feed, 10000)--10s喂一次狗

local LEDA = gpio.setup(24, 0, gpio.PULLUP) -- PB8输出模式
local LEDB = gpio.setup(25, 0, gpio.PULLUP) -- PB9输出模式
local LEDC = gpio.setup(26, 0, gpio.PULLUP) -- PB10输出模式

-- local LEDA = gpio.setup(16, 0, gpio.PULLUP) -- PB0输出模式
-- local LEDB = gpio.setup(17, 0, gpio.PULLUP) -- PB1输出模式
-- local LEDC = gpio.setup(18, 0, gpio.PULLUP) -- PB2输出模式

sys.taskInit(function()
    wdt.init(15000)
    sys.timerLoopStart(wdt.feed, 10000)
    local count = 0
    local uid = mcu.unique_id() or ""
    while 1 do
        -- 一闪一闪亮晶晶
        LEDA(count & 0x01 == 0x01 and 1 or 0)
        LEDB(count & 0x02 == 0x02 and 1 or 0)
        LEDC(count & 0x03 == 0x03 and 1 or 0)
        log.info("gpio", "Go Go Go", uid:toHex(), count)
        sys.wait(1000)
        count = count + 1
    end
end)

-- 用户代码已结束---------------------------------------------
-- 结尾总是这一句
sys.run()
-- sys.run()之后后面不要加任何语句!!!!!
