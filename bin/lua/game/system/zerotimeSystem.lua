--每天23:59:59清理任务
local zerotimeSystem = { is_start = false}

function zerotimeSystem:zerotime()
    for k,v in pairs(RoleMgr.rid) do
        v:resetdata()
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

