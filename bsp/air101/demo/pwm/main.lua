
-- LuaTools需要PROJECT和VERSION这两个信息
PROJECT = "mypwm"
VERSION = "1.0.0"

-- sys库是标配
_G.sys = require("sys")

-- PWM4 --> PA7-7
-- PWM3 --> PB3-19
-- PWM2 --> PB2-18
-- PWM1 --> PB1-17
-- PWM0 --> PB0-16

sys.taskInit(function()
    wdt.init(15000)
    sys.timerLoopStart(wdt.feed, 10000)
    while 1 do
        -- 仿呼吸灯效果
        log.info("pwm", ">>>>>")
        for i = 10,1,-1 do 
            pwm.open(1, 1000, i*9) -- 频率1000hz, 占空比0-100
            sys.wait(200 + i*10)
        end
        for i = 10,1,-1 do 
            pwm.open(4, 1000, 100 - i*9)
            sys.wait(200 + i*10)
        end
        sys.wait(5000)
    end
end)

-- 用户代码已结束---------------------------------------------
-- 结尾总是这一句
sys.run()
-- sys.run()之后后面不要加任何语句!!!!!
