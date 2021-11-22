--每天23:59:59清理任务
local zerotimeSystem = {}

function zerotimeSystem:uplevel()
    for k,v in pairs(RoleMgr) do
        if type(v) == "table" then
            --遍历角色
            v:resetdata()
        end
    end
    TimerMgr:bind(24 * 60 * 60 * 1000,false,zerotimeSystem,'zerotime',self)
end

function zerotimeSystem:start()
    if self.is_start == false then
        TimerMgr:bind(DistanceZeroTimeSeconds() * 1000,false,zerotimeSystem,'zerotime',self)
        self.is_start = true
    end
end


return zerotimeSystem

