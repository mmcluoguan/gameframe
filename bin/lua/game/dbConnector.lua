local baseNet =require ("lua/common/baseNet")
local role = require ("lua/game/role")
local pb = require("pb")

local dbConnector = {}
--继承
setmetatable(dbConnector,baseNet)
baseNet.__index = baseNet

function dbConnector:init()
    baseNet:init(self);
end

function dbConnector:clean()
    baseNet:clean(self);
end

--加载1条hash数据结果
function dbConnector:loaddata_from_dbvisit_s(msgid,msgdata,routing)
    local gameClientFd = routing:top():fd()
    routing:pop()
    local gameClient = acceptMgr:find(gameClientFd)
    if gameClient ~= nil then
        local msgs = pb.decode("frmpub.protocc.loaddata_from_dbvisit_s", msgdata)
        local temp = split(msgs.tag,',')
        if msgs.result == 0 then
            local msgname,msgtable
            if temp[1] == 'roledata' then
                msgname,msgtable = self:loadrole_client_gate_c(tonumber(temp[2]),msgs.fields,gameClientFd);
            end
            if msgname ~= nil then
                gameClient:send(msgname,msgtable,routing)
            end
        else
            log("加载1条hash数据失败 tag:",msgs.tag)
        end
    else
        log("网关 fd:",gameClientFd," 已断开连接")
    end
end

--加载n条hash数据结果
function dbConnector:loaddata_more_from_dbvisit_s(msgid,msgdata,routing)
    local gameClientFd = routing:top():fd()
    routing:pop()
    local gameClient = acceptMgr:find(gameClientFd)
    if gameClient ~= nil then
        local msgs = pb.decode("frmpub.protocc.loaddata_more_from_dbvisit_s", msgdata)
        local temp = split(msgs.tag,',')
        local msgname,msgtable
        if temp[1] == 'goodsdata' then
            msgname,msgtable = self:loadgoods_client_gate_c(tonumber(temp[2]),msgs.objs);
        end
        if msgname ~= nil then
            gameClient:send(msgname,msgtable,routing)
        end
    else
        log("网关 fd:",gameClientFd," 已断开连接")
    end
end

--加载角色结果
function dbConnector:loadrole_client_gate_c(roleid,fields,gameClientFd)
    local msgtable = {
        result = 0,
    }
    local roleObj = roleMgr:find(roleid)
    if roleObj == nil then
        roleObj = role:new(roleid,gameClientFd)
        roleMgr:add(roleObj)
    end
    roleObj.online = true
    for i = 1, #fields do
        if fields[i].key == '_id' then
            roleObj.id = tonumber(fields[i].value)
            msgtable.roleid = roleObj.id
        elseif fields[i].key == 'accountid' then
            roleObj.accountid = fields[i].value
            msgtable.aid = roleObj.accountid
        elseif fields[i].key == 'level' then
            roleObj.level = tonumber(fields[i].value)
            msgtable.level = roleObj.level
        end
    end
    log("在db中获取角色数据 roleid:",roleObj.id)
    return 'loadrole_client_gate_s',msgtable
end

--加载角色物品结果
function dbConnector:loadgoods_client_gate_c(roleid,objs)
    local roleObj = roleMgr:find(roleid)
    assert(roleObj,"没有角色 roleid:" .. roleid)
    for i = 1, #objs do
        roleObj.goods[i] = {}
        local goodsid = nil
        for j = 1, #objs[i].fields do
            local key = objs[i].fields[j].key
            local value = objs[i].fields[j].value
            if key == '_id' then
                goodsid = tonumber(value)   
                roleObj.goods[i].id = goodsid           
            elseif key == 'cfgid' then
                roleObj.goods[i].cfgid = tonumber(value)
            elseif key == 'num' then
                roleObj.goods[i].num = tonumber(value)
            end
        end
        roleObj.goods[goodsid] = roleObj.goods[i]
        roleObj.goods[i] = nil
    end
    local goodsdata = {}
    local i = 1
    for k,v in pairs(roleObj.goods) do
        table.insert(goodsdata,i,v)
        i = i + 1
    end
    log("在db中获取角色物品数据 roleid:",roleObj.id)
    return 'loadgoods_client_gate_s',{ goods = goodsdata}
end

return dbConnector;