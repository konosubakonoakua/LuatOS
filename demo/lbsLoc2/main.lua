
-- LuaTools需要PROJECT和VERSION这两个信息
PROJECT = "lbsLoc2demo"
VERSION = "1.0.0"


-- sys库是标配
sys = require("sys")

local lbsLoc2 = require("lbsLoc2")

sys.taskInit(function()
    sys.waitUntil("IP_READY", 3000)
    while module do -- 没有mobile库就没有基站定位
        local lat, lng, t = lbsLoc2.request(5000)
        -- local lat, lng, t = lbsLoc2.request(5000, "bs.openluat.com")
        log.info("lbsLoc2", lat, lng, (json.encode(t or {})))
        sys.wait(1000)
    end
end)

-- 用户代码已结束---------------------------------------------
-- 结尾总是这一句
sys.run()
-- sys.run()之后后面不要加任何语句!!!!!
