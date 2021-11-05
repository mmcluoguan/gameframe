local baseNet = require ("lua/common/baseNet")
ConnectorMgr = require ("lua/world/connectorMgr")

local worldClient = {}
--继承
setmetatable(worldClient,baseNet)
baseNet.__index = baseNet

function worldClient:init()
    baseNet:init(self);
end

function worldClient:clean()
    baseNet:clean(self);
end

return worldClient;