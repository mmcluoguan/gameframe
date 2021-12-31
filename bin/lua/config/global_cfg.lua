local items =
{
	{ id=1, value=10, desc="每日抽奖次数",},
}

local idItems =
{
	[1]=items[1],
}


local data = { Items = items, IdItems = idItems, reloaddata = 1 }
function data:getById(id)
    return self.IdItems[id]
end
return data
