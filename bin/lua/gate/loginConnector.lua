local baseNet =require ("lua/common/baseNet")

local loginConnector = {}
--继承
setmetatable(loginConnector,baseNet)
baseNet.__index = baseNet

function loginConnector:init()
    baseNet:init(self);
end

function loginConnector:clean()
    baseNet:clean(self);
end

return loginConnector;