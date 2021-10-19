local items = 
{
    { Id = 1, KeyName = "旋风斩", Pow = 100, Level = 1, Effect = {1001,1002}, },
    { Id = 2, KeyName = "冲击波", Pow = 120, Level = 2, Effect = {}, },
}

local idItems = 
{
    [1] = items[1],
    [2] = items[2],
}

local keyItems = 
{
    ["旋风斩"] = items[1],
    ["冲击波"] = items[2],
}


function __RELOAD()
    local data = { Items = items, IdItems = idItems, KeyItems = keyItems, }
    function data:getById(id)
        return self.IdItems[id]
    end
    function data:getByKey(key)
        return self.KeyItems[key]
    end
    return data
end
return __RELOAD()
