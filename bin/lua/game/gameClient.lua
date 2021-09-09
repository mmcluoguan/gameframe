local pb = require("pb")
local baseNet = require ("lua/common/baseNet")
local connectorMgr = require ("lua/game/connectorMgr")
local roleMgr =  require ("lua/game/roleMgr")
local timerMgr = require("lua/common/timerMgr")

local gameClient = {}
--继承
setmetatable(gameClient,baseNet)
baseNet.__index = baseNet

function gameClient:init()
    baseNet:init(self);
end

function gameClient:clean()
    baseNet:clean(self);
end

--没有处理的消息，转移给关联的角色处理
function gameClient:defaultHandle(msgid,msgname,data,routing)

    if #data ~= 0 then
        --解包roleid
        local msgtable = pb.decode("frmpub.protocc." .. msgname, data)
        if msgtable.roleid ~= nil then
            local role = roleMgr:find(msgtable.roleid)
            if role ~= nil then
                --socket与角色关联
                role.fd = self.id
                role:handle_message(msgname,data,routing)
            else
                log("没有角色 roleid:",msgtable.roleid)
            end
        else
            baseNet:defaultHandle(msgid,msgname,data,routing)
        end
    else
        baseNet:defaultHandle(msgid,msgname,data,routing)
    end
end

function gameClient:regMsg()
    baseNet:regMsg(self);
    self:regMsgEx('createrole_client_gate_c')
    self:regMsgEx('loadrole_client_gate_c')
    self:regMsgEx('loadgoods_client_gate_c')
    self:regMsgEx('clioffline_gate_all_c')
    self:regMsgEx('reconnect_client_gate_s')
end

--延迟销毁角色数据
function gameClient:cleanRoleObj(role)
    if role.online == false then
        roleMgr:remove(role.id)
    end
end

--创建角色
function gameClient:createrole_client_gate_c(msgid,msgdata,routing)
    local msgtable = pb.decode("frmpub.protocc.createrole_client_gate_c", msgdata)
    local msgdata={}
    msgdata.result = 0;
    if roleMgr:findby_accountid(msgtable.aid) == nil then
        --创建角色
        local role = require("lua/game/role")
        local roleObj = role:new(newid(),self.id)
        roleObj.accountid = msgtable.aid
        roleObj.level = random(1,100)
        local savedata = {
            cache_key = 'role_' .. roleObj.id,
            fields = {
                { key = '_id', value = tostring(roleObj.id),},
                { key = 'accountid', value = tostring(roleObj.accountid),},
                { key = 'level', value = tostring(roleObj.level),},
                { key = 'star', value = tostring(roleObj.star),},
            }
        }
        connectorMgr:dbConnector():send('insertdata_to_dbvisit_c',savedata)
        --默认初始化3个物品
        for i = 1, 3 do
            local item = {
                id = newid(),
                cfgid = random(10,20),
                num = random(10,20),
            }
            table.insert(roleObj.goods,#roleObj.goods + 1,item)
            local savedata = {
                cache_key = 'goods_' .. item.id .. "_" .. roleObj.id,
                fields = {
                    { key = '_id', value = tostring(item.id),},
                    { key = 'cfgid', value = tostring(item.cfgid),},
                    { key = 'num', value = tostring(item.num),},
                }
            }
            connectorMgr:dbConnector():send('insertdata_to_dbvisit_c',savedata)
        end
        msgdata.aid = roleObj.accountid
        msgdata.roleid = roleObj.id
    else
        msgdata.result = 2;
    end
    self:send("createrole_client_gate_s",msgdata,routing)
end

--加载角色数据
function gameClient:loadrole_client_gate_c(msgid,msgdata,routing)
    local msgtable = pb.decode("frmpub.protocc.loadrole_client_gate_c", msgdata)
    local role = roleMgr:find(msgtable.roleid)
    if role ~= nil then
        --在本地内存中获取
        log("在本地内存中获取角色数据 roleid:",role.id)
        role.online = true
        local roledata = {
            roleid = role.id,
            aid = role.accountid,
            level = role.level,
        };
        self:send("loadrole_client_gate_s",roledata,routing)
    else
        --转发消息到db获取角色数据
        local enve = Envelope_CPP.new()
        enve:fd(self.id)
        enve:addr(self.cpp_socket:remote_addr())
        routing:push(enve)
        local roledata = {
            cache_key = "role_" .. msgtable.roleid,
            tag = "roledata," .. msgtable.roleid,
            fields = {
                {key = '_id', value = '',},
                {key = 'level', value = '',},
                {key = 'accountid', value = '',},
            },
        };
        connectorMgr:dbConnector():send('loaddata_from_dbvisit_c',roledata,routing)
    end
end

--加载角色物品数据
function gameClient:loadgoods_client_gate_c(msgid,msgdata,routing)
    local msgtable = pb.decode("frmpub.protocc.loadgoods_client_gate_c", msgdata)
    local role = roleMgr:find(msgtable.roleid)
    if role ~= nil and #role.goods ~= 0 then
        --在本地内存中获取
        log("在本地内存中获取角色物品数据 roleid:",role.id)
        self:send("loadgoods_client_gate_s",{ goods = role.goods },routing)
    else
        --转发消息到db获取角色数据
        local enve = Envelope_CPP.new()
        enve:fd(self.id)
        enve:addr(self.cpp_socket:remote_addr())
        routing:push(enve)
        local goodsdata = {
            tag = "goodsdata," .. msgtable.roleid,
            condition = "goods_*_" .. msgtable.roleid,
            fields = {
                {key = '_id', value = '',},
                {key = 'cfgid', value = '',},
                {key = 'num', value = '',},
            },
        }
        connectorMgr:dbConnector():send('loaddata_more_from_dbvisit_c',goodsdata,routing)
    end
end

--玩家下线
function gameClient:clioffline_gate_all_c(msgid,msgdata,routing)
    local msgtable = pb.decode("frmpub.protocc.clioffline_gate_all_c", msgdata)
    local role = roleMgr:findby_accountid(msgtable.aid)
    if role ~= nil then
        --延迟10s销毁角色数据
        role.online = false;
        timerMgr:unbind(role.destroy_timerid)
        role.destroy_timerid = timerMgr:bind(1000*10,gameClient.cleanRoleObj,{self,role},false)
    end
end

--玩家断线重连成功
function gameClient:reconnect_client_gate_s(msgid,msgdata,routing)
    local msgtable = pb.decode("frmpub.protocc.reconnect_client_gate_s", msgdata)
    local role = roleMgr:findby_accountid(msgtable.aid)
    if role ~= nil then
        role.online = true
        log("玩家断线重连成功 id:",role.id)
    end
end

return gameClient;