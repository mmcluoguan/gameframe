local pb = require("pb")
local baseNet = require ("lua/common/baseNet")
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
    assert(routing:size() ~= 0,'defaultHandle 没有路由信息');
    local clientfd = routing:top():fd();
    local role = RoleMgr:findby_clientfd(clientfd)
    if role ~= nil then
        --socket与角色关联
        role.gatefd = self.id
        role.clientfd = clientfd
        if role:handle_message(msgname,data,routing) == false then
            baseNet:defaultHandle(msgid,msgname,data,routing)
        end
    else
        baseNet:defaultHandle(msgid,msgname,data,routing)
        log("没有角色 clientfd:",clientfd)
        self:send_errcode("GAME_ROLE_NOT_EXIST",'游戏服内没有此连接的角色,可能由于游戏服重启导致',routing)
    end
end

--延迟销毁角色数据
function gameClient:cleanRoleObj(role)
    if role.online == false then
        RoleMgr:remove(role.id)
    end
end

--创建角色
function gameClient:createrole_client_gate_c(msgid,msgdata,routing)
    local msgtable = pb.decode("frmpub.protocc.createrole_client_gate_c", msgdata)
    local msgdata={}
    msgdata.result = 0;
    if RoleMgr:findby_accountid(msgtable.aid) == nil then
        --创建角色
        local role = require("lua/game/role")
        local roleObj = role:new(newid(),self.id)
        roleObj.accountid = msgtable.aid
        roleObj.level = random(1,100)
        local initdata = {
            cache_key = 'role_' .. roleObj.id,
            opertype = 0,
            fields = {
                { key = '_id', value = tostring(roleObj.id),},
                { key = 'accountid', value = tostring(roleObj.accountid),},
                { key = 'level', value = tostring(roleObj.level),},
                { key = 'star', value = tostring(roleObj.star),},
            }
        }
        ConnectorMgr:dbConnector():send('insertdata_to_dbvisit_c',initdata)
        --默认初始化3个物品
        -- for i = 1, 3 do
        --     local item = {
        --         id = newid(),
        --         cfgid = random(10,20),
        --         num = random(10,20),
        --     }
        --     table.insert(roleObj.goods,#roleObj.goods + 1,item)
        --     local savedata = {
        --         cache_key = 'goods_' .. item.id .. "_" .. roleObj.id,
        --         opertype = 0,
        --         fields = {
        --             { key = '_id', value = tostring(item.id),},
        --             { key = 'cfgid', value = tostring(item.cfgid),},
        --             { key = 'num', value = tostring(item.num),},
        --             { key = 'roleid', value = tostring(roleObj.id),},
        --         }
        --     }
        --     connectorMgr:dbConnector():send('insertdata_to_dbvisit_c',savedata)
        -- end
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
    local role = RoleMgr:find(msgtable.roleid)
    if role ~= nil then
        --在本地内存中获取
        log("在本地内存中获取角色数据 roleid:",role.id)
        role:copyrouting(routing)
        role.online = true
        role.gatefd = self.id
        assert(routing:size() ~= 0,'loadrole_client_gate_c 没有路由信息');
        role.clientfd = routing:top():fd()
        local roledata = {
            roleid = role.id,
            aid = role.accountid,
            level = role.level,
            gold = role.gold,
            diamond = role.diamond
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
            opertype = 0,
            fields = {
                {key = '_id', value = '',},
                {key = 'level', value = '0',},
                {key = 'accountid', value = '',},
                {key = 'gold', value = '0',},
                {key = 'diamond', value = '0',},
                {key = 'unline_time', value = '0',},
            },
        };
        ConnectorMgr:dbConnector():send('loaddata_from_dbvisit_c',roledata,routing)
    end
end

--加载角色物品数据
function gameClient:loadgoods_client_gate_c(msgid,msgdata,routing)
    assert(routing:size() ~= 0,'loadgoods_client_gate_c 没有路由信息');
    local clientfd = routing:top():fd()
    local role = RoleMgr:findby_clientfd(clientfd)
    assert(role,'角色不存在,必须先获取角色基础数据')
    if get_tablekey_size(role.goods) ~= 0 then
        --在本地内存中获取
        log("在本地内存中获取角色物品数据 roleid:",role.id)
        local goodsdata = {}
        local i = 1
        for k,v in pairs(role.goods) do
            table.insert(goodsdata,i,v)
            i = i + 1
        end
        self:send("loadgoods_client_gate_s",{ goods = goodsdata },routing)
    else
        --转发消息到db获取角色数据
        local enve = Envelope_CPP.new()
        enve:fd(self.id)
        enve:addr(self.cpp_socket:remote_addr())
        routing:push(enve)
        local goodsdata = {
            tag = "goodsdata," .. role.id,
            condition = "goods_*_" .. role.id,
            sort = "",
            limit = 0,
            opertype = 0,
            fields = {
                {key = '_id', value = '',},
                {key = 'cfgid', value = '',},
                {key = 'num', value = '',},
            },
        }
        ConnectorMgr:dbConnector():send('loaddata_more_from_dbvisit_c',goodsdata,routing)
    end
end

--玩家下线
function gameClient:clioffline_gate_all_c(msgid,msgdata,routing)
    local msgtable = pb.decode("frmpub.protocc.clioffline_gate_all_c", msgdata)
    local role = RoleMgr:findby_accountid(msgtable.aid)
    if role ~= nil then
        --延迟10s销毁角色数据
        log('角色下线,延迟10s销毁角色数据 roleid:',role.id)
        role.online = false;
        TimerMgr:unbind(role.destroy_timerid)
        role.destroy_timerid = TimerMgr:bind(1000*10,false,gameClient,'cleanRoleObj',self,role)
    end
end

--玩家断线重连成功
function gameClient:reconnect_client_gate_s(msgid,msgdata,routing)
    local msgtable = pb.decode("frmpub.protocc.reconnect_client_gate_s", msgdata)
    local role = RoleMgr:findby_accountid(msgtable.aid)    
    if role ~= nil then
        role.online = true
        role:copyrouting(routing)
        log("玩家断线重连成功 id:",role.id)
    end
end

--gm命令
function gameClient:gmorder_client_gate_c(msgid,msgdata,routing)
    local msgtable = pb.decode("frmpub.protocc.gmorder_client_gate_c", msgdata)
    local gmtable = {
        result = 0,
        order = msgtable.order,
        desc = '成功',
    };
    
    if msgtable.order == 'addgoods' then
        GmSystem:addgoods(msgtable,gmtable,routing)
    elseif msgtable.order == 'delgoods' then
        GmSystem:delgoods(msgtable,gmtable,routing)
    else
        gmtable.result = 2;
        gmtable.desc = '非法的命令'
    end
    self:send("gmorder_client_gate_s",gmtable,routing)
end

--广播公告信息列表
function gameClient:notice_info_list_clent_gate_c(msgid,msgdata,routing)
    --转发消息到db获取广播公告信息列表
    local enve = Envelope_CPP.new()
    enve:fd(self.id)
    enve:addr(self.cpp_socket:remote_addr())
    routing:push(enve)
    local noticedata = {
        tag = "noticedata",
        condition = "notice_*",
        sort = "time desc",
        limit = 10,
        opertype = 1,
        fields = {
            {key = 'info', value = '',},
            {key = 'time', value = '',},
        },
    }
    ConnectorMgr:dbConnector():send('loaddata_more_from_dbvisit_c',noticedata,routing)
end

return gameClient;