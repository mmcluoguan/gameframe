local baseNet =require ("lua/common/baseNet")
local dynamicData = require("lua/game/data/dynamicData")
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
    local msgs = pb.decode("frmpub.protocc.loaddata_from_dbvisit_s", msgdata)
    local temp = split(msgs.tag,',')
    if routing:size() ~= 0 then
        local gameClientFd = routing:top():fd()
        routing:pop()
        local gameClient = AcceptMgr:find(gameClientFd)
        if gameClient ~= nil then            
            if msgs.result == 0 then
                local msgname,msgtable
                if temp[1] == 'roledata' then
                    msgname,msgtable = self:loadrole_client_gate_c(tonumber(temp[2]),msgs.fields,gameClientFd,routing);
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
    else

    end
end

--加载n条hash数据结果
function dbConnector:loaddata_more_from_dbvisit_s(msgid,msgdata,routing)
    local msgs = pb.decode("frmpub.protocc.loaddata_more_from_dbvisit_s", msgdata)
    local temp = split(msgs.tag,',')
    if routing:size() ~= 0 then
        local gameClientFd = routing:top():fd()
        routing:pop()
        local gameClient = AcceptMgr:find(gameClientFd)
        if gameClient ~= nil then            
            local msgname,msgtable
            if temp[1] == 'goodsdata' then
                msgname,msgtable = self:loadgoods_client_gate_c(tonumber(temp[2]),msgs.objs);
            elseif temp[1] == 'noticedata' then
                msgname,msgtable = self:notice_info_list_clent_gate_c(msgs.objs);
            elseif temp[1] == 'emailsdata' then
                msgname,msgtable = self:loademails_client_gate_c(tonumber(temp[2]),msgs.objs);
            end
            if msgname ~= nil then
                gameClient:send(msgname,msgtable,routing)
            end
        else
            log("网关 fd:",gameClientFd," 已断开连接")
        end
    else
        if temp[1] == 'loadsysemail' then
            EmailSystem:loaded(msgs.objs);
        end
    end
end

--加载角色结果
function dbConnector:loadrole_client_gate_c(roleid,fields,gameClientFd,routing)
    local msgtable = {
        result = 0,
    }
    local roleObj = RoleMgr:find(roleid)
    if roleObj == nil then
        roleObj = role:new(roleid,gameClientFd)
        RoleMgr:add(roleObj)
    end
    roleObj:copyrouting(routing)
    assert(routing:size() ~= 0,'loadrole_client_gate_c 没有路由信息');
    roleObj.clientfd = routing:top():fd()
    roleObj.online = true
    roleObj.online_time = os.time()
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
        elseif fields[i].key == 'gold' then
            roleObj.gold = tonumber(fields[i].value)
            msgtable.gold = roleObj.gold
        elseif fields[i].key == 'diamond' then
            roleObj.diamond = tonumber(fields[i].value)
            msgtable.diamond = roleObj.diamond
        elseif fields[i].key == 'unline_time' then
            roleObj.unline_time = tonumber(fields[i].value)
        end
    end
    log("在db中获取角色数据 roleid:",roleObj.id)
    return 'loadrole_client_gate_s',msgtable
end

--加载角色物品结果
function dbConnector:loadgoods_client_gate_c(roleid,objs)
    local roleObj = RoleMgr:find(roleid)
    assert(roleObj,"没有角色 roleid:" .. roleid)
    for i = 1, #objs do
        roleObj.goods[i] = dynamicData.goodsdata:new()
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

--广播公告信息列表
function dbConnector:notice_info_list_clent_gate_c(objs)
    local noticedata ={}
    for i = 1, #objs do
        local notice = {}
        for j = 1, #objs[i].fields do
            local key = objs[i].fields[j].key
            local value = objs[i].fields[j].value
            if key == 'info' then
                notice.info = value
            elseif key == 'time' then
                notice.time = tonumber(value)
            end
        end
        table.insert(noticedata,notice)
    end
    log("在db中获取广播公告信息列表 len:",#noticedata)
    return 'notice_info_list_clent_gate_s',{ datas = noticedata}
end

--加载角色邮件列表
function dbConnector:loademails_client_gate_c(roleid,objs)
    local roleObj = RoleMgr:find(roleid)
    assert(roleObj,"没有角色 roleid:" .. roleid)
    for i = 1, #objs do
        roleObj.emails[i] = dynamicData.emaildata:new()
        local emailid = nil
        for j = 1, #objs[i].fields do
            local key = objs[i].fields[j].key
            local value = objs[i].fields[j].value
            if key == '_id' then
                emailid = tonumber(value)   
                roleObj.emails[i].id = emailid           
            elseif key == 'is_read' then
                roleObj.emails[i].is_read = stobool(value)
            elseif key == 'is_receive' then
                roleObj.emails[i].is_receive = stobool(value)
            end
        end
        roleObj.emails[emailid] = roleObj.emails[i]
        roleObj.emails[i] = nil
    end    
    log("在db中获取角色邮件列表 roleid:",roleObj.id)
    return roleObj:emails_data()
end

return dbConnector;