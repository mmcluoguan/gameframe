local baseNet =require ("lua/common/baseNet")

local gameConnector = {}
--继承
setmetatable(gameConnector,baseNet)
baseNet.__index = baseNet

function gameConnector:init()
    baseNet:init(self);
end

function gameConnector:clean()
    baseNet:clean(self);
end

return gameConnector;