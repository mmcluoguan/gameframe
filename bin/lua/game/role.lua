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
    --消息映射
    o.msg_map_ = {}
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

    o:regMsg("setlevel_client_gate_c")
    return o
end

function role:regMsg(msgname)
    self.msg_map_[msgname] = self[msgname]
end

function role:clean()
    log("销毁角色 id:",self.id)
end

function role:send(msg, msgdata,routing)
    local gameclient = acceptMgr:find(self.fd)
    if gameclient ~= nil then
       return gameclient:send(msg, msgdata,routing)
    else
       log("网关已断开连接")
    end
end

--分发消息
function role:handle_message(msgname,data,routing)
    if(self.msg_map_[msgname] ~= nil) then
        return self.msg_map_[msgname](self,data,routing)
    end
end

--重置角色等级
function role:setlevel_client_gate_c(msgname,data,routing)
    self.level = 1
    --获得物品
    local item = {
        id = newid(),
        cfgid = random(10,20),
        num = random(10,20),
    }
    table.insert(self.goods,#self.goods,item)
    log("重置角色等级 id:",self.id,"获得物品 goodsid:",item.id)
    local savedata = {
        cache_key = 'goods_' .. item.id .. "_" .. self.id,
        fields = {
            { key = '_id', value = tostring(item.id),},
            { key = 'cfgid', value = tostring(item.cfgid),},
            { key = 'num', value = tostring(item.num),},
        }
    }
    local connectorMgr = require ("lua/game/connectorMgr")
    connectorMgr.dbConnector:send('insertdata_to_dbvisit_c',savedata)
end

return role