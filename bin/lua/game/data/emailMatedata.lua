--附件表
local annexdata = {}

function annexdata:new()
    local o = {}
    setmetatable(o,self)
    self.__index = self

    o.gold = nil
    o.diamond = nil
    --物品列表
    o.goods = nil

    return o
end

--邮件表
local emaildata = {}

function emaildata:new()
    local o = {}
    setmetatable(o,self)
    self.__index = self

    o.id = nil
    o.title = nil
    o.info = nil
    o.type = nil
    o.time = nil
    o.annex = nil --附件
    
    return o
end

local temp = {
    emaildata = emaildata,
    annexdata = annexdata,
}

return temp