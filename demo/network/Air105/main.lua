
-- LuaTools需要PROJECT和VERSION这两个信息
PROJECT = "w5500_network"
VERSION = "1.0.0"

-- sys库是标配
_G.sys = require("sys")
require "net_test"
--下面演示用阻塞方式做串口透传远程服务器，简单的串口DTU，用串口3
demo1(3)
-- 用户代码已结束---------------------------------------------
-- 结尾总是这一句
sys.run()
-- sys.run()之后后面不要加任何语句!!!!!
