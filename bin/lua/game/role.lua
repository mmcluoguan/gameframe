--角色表
local role = {}

function role:new(id,fd)
    local o = {}
    setmetatable(o,self)
    self.__index = self

    log("创建角色 id:",id)
    --角色id
    o.id = id
    --账号id
    o.accountid = nil
    --socket标识
    o.fd = fd
    --路由信息
    o.routing = nil
    --是否在线,当false时会延迟删除内存数据
    o.online = true
    --等级
    o.level = 1
    --物品
    o.goods = {}
    --销毁计时器id
    o.destroy_timerid = 0
    --星级
    o.star = 0

    return o
end

function role:clean()
    log("销毁角色 id:",self.id)
end

function role:send(msg, msgdata,routing)
    if routing == nil then
        routing = self.routing
    end
    local gameclient = acceptMgr:find(self.fd)
    if gameclient ~= nil then
       return gameclient:send(msg, msgdata,routing)
    else
       log("网关已断开连接")
    end
end

--分发消息
function role:handle_message(msgname,data,routing)
    self.routing = routing
    local fun = self[msgname]
    if fun == nil then
        return false
    else
        return self[msgname](self,data,routing)
    end
end

--重置角色等级
function role:setlevel_client_gate_c(msgname,data,routing)
    self.level = 1
end

return role