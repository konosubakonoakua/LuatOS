
-- LuaTools需要PROJECT和VERSION这两个信息
PROJECT = "repl"
VERSION = "1.0.0"

log.info("main", PROJECT, VERSION)

-- 一定要添加sys.lua !!!!
sys = require("sys")

sys.taskInit(function()
    if rtos.bsp() == "EC618" then
        uart.setup(uart.VUART_0)
        uart.on(uart.VUART_0, "recv", function(id, len)
            repeat
                s = uart.read(id, len)
                if s and #s > 0 then -- #s 是取字符串的长度
                    repl.push(s)
                end
            until s == ""
        end)
    end

end)

-- 用户代码已结束---------------------------------------------
-- 结尾总是这一句
sys.run()
-- sys.run()之后后面不要加任何语句!!!!!

