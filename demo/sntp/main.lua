
-- LuaTools需要PROJECT和VERSION这两个信息
PROJECT = "sntpdemo"
VERSION = "1.0.0"

--[[
本demo需要mqtt库, 大部分能联网的设备都具有这个库
mqtt也是内置库, 无需require
]]

-- sys库是标配
_G.sys = require("sys")
--[[特别注意, 使用mqtt库需要下列语句]]
_G.sysplus = require("sysplus")


-- Air780E的AT固件默认会为开机键防抖, 导致部分用户刷机很麻烦
if rtos.bsp() == "EC618" and pm and pm.PWK_MODE then
    pm.power(pm.PWK_MODE, false)
end

-- 统一联网函数
sys.taskInit(function()
    local device_id = mcu.unique_id():toHex()
    -----------------------------
    -- 统一联网函数, 可自行删减
    ----------------------------
    if wlan and wlan.connect then
        -- wifi 联网, ESP32系列均支持
        local ssid = "uiot"
        local password = "12345678"
        log.info("wifi", ssid, password)
        -- TODO 改成自动配网
        -- LED = gpio.setup(12, 0, gpio.PULLUP)
        wlan.init()
        wlan.setMode(wlan.STATION) -- 默认也是这个模式,不调用也可以
        device_id = wlan.getMac():toHex()
        wlan.connect(ssid, password, 1)
    elseif mobile then
        -- Air780E/Air600E系列
        --mobile.simid(2) -- 自动切换SIM卡
        -- LED = gpio.setup(27, 0, gpio.PULLUP)
        device_id = mobile.imei()
    elseif w5500 then
        -- w5500 以太网, 当前仅Air105支持
        w5500.init(spi.HSPI_0, 24000000, pin.PC14, pin.PC01, pin.PC00)
        w5500.config() --默认是DHCP模式
        w5500.bind(socket.ETH0)
        -- LED = gpio.setup(62, 0, gpio.PULLUP)
    elseif socket or mqtt then
        -- 适配的socket库也OK
        -- 没有其他操作, 单纯给个注释说明
    else
        -- 其他不认识的bsp, 循环提示一下吧
        while 1 do
            sys.wait(1000)
            log.info("bsp", "本bsp可能未适配网络层, 请查证")
        end
    end
    -- 默认都等到联网成功
    sys.waitUntil("IP_READY")
    sys.publish("net_ready", device_id)
end)

sys.taskInit(function()
    -- 等待联网
    local ret, device_id = sys.waitUntil("net_ready")
    sys.wait(1000)
    -- 对于Cat.1模块, 移动/电信卡, 通常会下发基站时间,  那么sntp就不是必要的, 而联通卡通常不会下发, 就需要sntp了
    -- 对应ESP32系列模块, 固件默认也会执行sntp, 所以手动调用sntp也是可选的
    -- sntp内置了几个常用的ntp服务器, 也支持自选服务器
    while 1 do
        -- 使用内置的ntp服务器地址, 包括阿里ntp
        socket.sntp()
        -- 自定义ntp地址
        -- socket.sntp("ntp.aliyun.com")
        -- 通常只需要几百毫秒就能成功
        local ret = sys.waitUntil("NTP_UPDATE", 5000)
        if ret then
            -- 以下是获取/打印时间的演示,注意时区问题
            log.info("sntp", "时间同步成功", "本地时间", os.date())
            log.info("sntp", "时间同步成功", "UTC时间", os.date("!%c"))
            log.info("sntp", "时间同步成功", "RTC时钟(UTC时间)", json.encode(rtc.get()))
            -- 正常使用, 一小时一次, 已经足够了, 甚至1天一次也可以
            -- sys.wait(3600000) 
            -- 这里为了演示, 用1分钟一次
            sys.wait(60000)
        else
            log.info("sntp", "时间同步失败")
            sys.wait(60000) -- 1分钟后重试
        end
    end
end)

-- 打印内存状态并回收
-- sys.taskInit(function ()
--     while true do
--         sys.wait(3000)
--         log.info("lua", rtos.meminfo())
--         log.info("sys", rtos.meminfo("sys"))
--         collectgarbage()
--         collectgarbage()
--     end
-- end)


-- 用户代码已结束---------------------------------------------
-- 结尾总是这一句
sys.run()
-- sys.run()之后后面不要加任何语句!!!!!