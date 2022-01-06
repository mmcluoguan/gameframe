local gmSystem = {}

function gmSystem:addgoods(msgtable,gmtable,routing)
    if #msgtable.args < 2 then
        gmtable.desc = '参数数量小于2'
    else            
        local cfgid = tonumber(msgtable.args[1])
        local num = tonumber(msgtable.args[2])
        if cfgid == nil or num == nil then
            gmtable.result = 2
            gmtable.desc = '错误的参数' .. msgtable.args[1] .. ',' .. msgtable.args[2]
        else
            local role = RoleMgr:find(msgtable.roleid)
            if role ~= nil then
                role.routing = routing
                if num > 0 then
                    if BagOperator:add_one_item(role,cfgid,num,Module_name.gm) == nil then
                        gmtable.result = 2
                        gmtable.desc = '没有物品配置id:' .. cfgid
                    end
                elseif num < 0 then
                    if BagOperator:sub_one_item(role,cfgid,num,Module_name.gm) == nil then
                        gmtable.result = 2
                        gmtable.desc = '没有物品配置id:' .. cfgid
                    end
                else
                    gmtable.result = 2
                    gmtable.desc = '错误的数量num:' .. num
                end
            else                    
                gmtable.result = 1;
                gmtable.desc = '没有角色' .. msgtable.roleid
            end
        end      
    end
end

function gmSystem:delgoods(msgtable,gmtable,routing)
    if #msgtable.args < 1 then
        gmtable.desc = '参数数量小于1'
    else
        local goods_id = tonumber(msgtable.args[1])
        if goods_id == nil then
            gmtable.result = 2
            gmtable.desc = '错误的参数' .. msgtable.args[1]
        else
            local role = RoleMgr:find(msgtable.roleid)
            if role ~= nil then
                if role.goods[goods_id] == nil then
                    gmtable.result = 3
                    gmtable.desc = '没有物品 id:' .. goods_id
                else
                    role.routing = routing
                    BagOperator:del_one_item(role,goods_id,Module_name.gm)
                end
            else                    
                gmtable.result = 1;
                gmtable.desc = '没有角色' .. msgtable.roleid
            end
        end     
    end
end

function gmSystem:changegold(msgtable,gmtable,routing)
    if #msgtable.args < 1 then
        gmtable.desc = '参数数量小于1'
    else
        local goldnum = tonumber(msgtable.args[1])
        if goldnum == nil then
            gmtable.result = 2
            gmtable.desc = '错误的参数' .. msgtable.args[1]
        else
            local role = RoleMgr:find(msgtable.roleid)
            if role ~= nil then
                role.gold = role.gold + goldnum
                role.routing = routing
                role:save('gold')
            else                    
                gmtable.result = 1;
                gmtable.desc = '没有角色' .. msgtable.roleid
            end
        end
    end
end

return gmSystem;