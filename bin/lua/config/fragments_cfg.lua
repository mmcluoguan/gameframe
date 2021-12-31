local items = 
{
	{ id=1000100, effect={1001,1002},},
	{ id=1010001, effect=nil,},
}

local idItems = 
{
	[1000100]=items[1],
	[1010001]=items[2],
}


local data = { Items = items, IdItems = idItems, reloaddata = 1 }
function data:getById(id)
    return self.IdItems[id]
end
return data
