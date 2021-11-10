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

return worldConnector;