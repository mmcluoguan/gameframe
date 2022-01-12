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
    self.lcb = {}
    self.mcb = {}
end

function dbConnector:clean()
    baseNet:clean(self);
end

--请求加载1条hash数据
function dbConnector:loaddata_one(data,routing,callback)
    local temp = split(data.tag,',')
    self.lcb[temp[1]] = callback
    self:send('loaddata_from_dbvisit_c',data,routing)
end

--请求加载n条hash数据
function dbConnector:loaddata_more(data,routing,callback)
    local temp = split(data.tag,',')
    self.mcb[temp[1]] = callback
    self:send('loaddata_more_from_dbvisit_c',data,routing)
end

--添加1条hash数据结果
function dbConnector:insertdata_to_dbvisit_s(msgid,msgdata,routing)
    
end

--更新1条hash数据结果
function dbConnector:updata_to_dbvisit_s(msgid,msgdata,routing)
    
end

--删除1条hash数据结果
function dbConnector:deletedata_to_dbvisit_s(msgid,msgdata,routing)
    
end

--加载1条hash数据结果
function dbConnector:loaddata_from_dbvisit_s(msgid,msgdata,routing)
    local msgs = pb.decode("frmpub.protocc.loaddata_from_dbvisit_s", msgdata)
    local cb = self.lcb[split(msgs.tag,',')[1]]
    cb(msgs,routing)
end

--加载n条hash数据结果
function dbConnector:loaddata_more_from_dbvisit_s(msgid,msgdata,routing)
    local msgs = pb.decode("frmpub.protocc.loaddata_more_from_dbvisit_s", msgdata)
    local cb = self.mcb[split(msgs.tag,',')[1]]
    assert(cb,'没有设置回调函数 tag:' .. msgs.tag)
    cb(msgs,routing)
end

return dbConnector;