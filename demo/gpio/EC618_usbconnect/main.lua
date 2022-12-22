
-- LuaTools需要PROJECT和VERSION这两个信息
PROJECT = "pwrkey_demo"
VERSION = "1.0.0"

log.info("main", PROJECT, VERSION)

-- sys库是标配
_G.sys = require("sys")


gpio.setup(33, function()
    log.info("usb", gpio.get(33))
end, gpio.PULLUP, gpio.BOTH)
gpio.debounce(33, 2000, 1)  --加入消抖是为了能看到输出
log.info("usb", gpio.get(33))
-- 用户代码已结束---------------------------------------------
-- 结尾总是这一句
sys.run()
-- sys.run()之后后面不要加任何语句!!!!!
