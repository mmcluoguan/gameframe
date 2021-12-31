local items =
{
	{ id=1010000, name="任务1", type=1, accept_cond=nil, comp_cond=nil, reward=nil,},
	{ id=1010001, name="任务2", type=1, accept_cond=nil, comp_cond=nil, reward=nil,},
}

local idItems =
{
	[1010000]=items[1],
	[1010001]=items[2],
}


local data = { Items = items, IdItems = idItems, reloaddata = 1 }
function data:getById(id)
    return self.IdItems[id]
end
return data
