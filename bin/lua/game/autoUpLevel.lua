--自动升级系统
local autoUpLevel = {}

function autoUpLevel:uplevel()
    for k,v in pairs(RoleMgr) do
        if type(v) == "table" then
            --遍历角色
            v.level = v.level + 1
            log("角色升级: roleid=",v.id," level=",v.level)
            --保存等级到db
            local savedata = {
                cache_key = 'role_' .. v.id,
                fields = {
                    { key = 'level', value = tostring(v.level),},
                }
            }
            ConnectorMgr:dbConnector():send('updata_to_dbvisit_c',savedata)
            if v.level % 5 == 0 then
                v.star = v.star + 1
                log("角色升星: roleid=",v.id," star=",v.star)
                --保存星级到db
                savedata.fields =  {{ key = 'star', value = tostring(v.star),}}
                ConnectorMgr:dbConnector():send('updata_to_dbvisit_c',savedata)
            end
        end
    end
end

function autoUpLevel:start()
    if self.is_start == false then
        self.timerid = TimerMgr:bind(5 * 1000,true,autoUpLevel,'uplevel',self)
        self.is_start = true
    end
end

function autoUpLevel:stop()
    TimerMgr:unbind(self.timerid)
    self.is_start = false
end


return autoUpLevel

