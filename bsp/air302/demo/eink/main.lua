
-- LuaTools需要PROJECT和VERSION这两个信息
PROJECT = "einkdemo"
VERSION = "1.0.0"

-- sys库是标配
_G.sys = require("sys")

--[[
本DEMO需要V0005或2020-12-14及之后的源码才支持
]]

--[[
显示屏为佳显 1.54寸,200x200,快刷屏
硬件接线
显示屏SPI          --> Air302 SPI
显示屏 Pin_BUSY        (GPIO18)
显示屏 Pin_RES         (GPIO7)
显示屏 Pin_DC          (GPIO9)
显示屏 Pin_CS          (GPIO16)
]]

function eink154_update()
    -- 设置视窗大小
    eink.setWin(200, 200, 0)

    eink.clear()

    sys.wait(1000)

    eink.print(30, 30, os.date(), 0, 12)

    --eink.printcn(20, 60, "中文", 0, 12)

    -- 刷屏幕
    eink.show()
end



sys.taskInit(function()

    -- 初始化必要的参数
    log.info("eink", "begin setup")
    eink.setup(1, 0)
    log.info("eink", "end setup")

    -- 稍微等一会,免得墨水屏没初始化完成
    sys.wait(1000)
    while 1 do
        log.info("e-paper 1.54", "Testing Go\r\n")
        eink154_update()
        log.info("e-paper 1.54", "Testing End\r\n")

        sys.wait(60000) -- 一分钟刷新一次
    end
end)

sys.run()
