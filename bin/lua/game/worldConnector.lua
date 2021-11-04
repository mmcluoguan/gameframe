local baseNet =require ("lua/common/baseNet")

local worldConnector = {}
--继承
setmetatable(worldConnector,baseNet)
baseNet.__index = baseNet

function __RELOAD(newchunk)
    local connectorMgr = require("lua/game/connectorMgr")
    local world = connectorMgr:worldConnector()
    if world ~= nil then
        world:regMsg()
    end
end

function worldConnector:init()
    baseNet:init(self);
end

function worldConnector:clean()
    baseNet:clean(self);
end

function worldConnector:regMsg()
    baseNet:regMsg(self);
end

return worldConnector;