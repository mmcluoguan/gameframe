local rapidjson = require('rapidjson')
local baseNet =require ("lua/common/baseNet")
local pb = require("pb")

local worldConnector = {}
--继承
setmetatable(worldConnector,baseNet)
baseNet.__index = baseNet

function worldConnector:init()
    baseNet:init(self);
end

function worldConnector:clean()
    baseNet:clean(self);
end

--通知区服服务器广播信息
function worldConnector:noticeserver_world_game_g(msgid,msgdata,routing)    
    log('通知区服服务器广播信息')
    local msgs = pb.decode("frmpub.protocc.noticeserver_world_game_g", msgdata)

    --更新数据库
    local id = newid()
    local noticedata = {
        cache_key = 'notice_' .. id,
        opertype = 1,
        fields = {
            { key = '_id', value = tostring(id),},
            { key = 'info', value = tostring(msgs.info),},
            { key = 'time', value = tostring(os.time()),},
        }
    }
    ConnectorMgr:dbConnector():send('insertdata_to_dbvisit_c',noticedata)

    --通知在线玩家
    RoleMgr:broadcast("notice_info_clent_gate_g",{info = msgs.info})
end

--新系统邮件
function worldConnector:sysemail_world_game_g(msgid,msgdata,routing)
    local msgs = pb.decode("frmpub.protocc.sysemail_world_game_g", msgdata)

    if msgs.rid == -1 then
        local annex_jsonstr = rapidjson.encode(setmetatable(msgs.annex, {__jsontype='object'}))
        --更新数据库
        local emaildata = {
            cache_key = 'sysemail_' .. msgs.id,
            opertype = 2,
            fields = {
                { key = '_id', value = tostring(msgs.id),},
                { key = 'title', value = tostring(msgs.title),},
                { key = 'info', value = tostring(msgs.info),},
                { key = 'type', value = tostring(msgs.type),},
                { key = 'time', value = tostring(msgs.time),},
                { key = 'annex', value = tostring(annex_jsonstr),},
            }
        }
        ConnectorMgr:dbConnector():send('insertdata_to_dbvisit_c',emaildata)
            
        --邮件系统添加新邮件
        EmailSystem:add_new(msgs)

        --通知在线玩家有新邮件
        for k,v in pairs(RoleMgr) do
            if type(v) == "table" then
                EmailOperator:add_one_email(v,msgs.id)
            end
        end   
    else
       --通知特定在线玩家有新邮件
       local role = RoleMgr:find(msgs.rid)
       if role ~= nil then
            EmailOperator:add_one_email(role,msgs.id)
       end
    end
end

return worldConnector;