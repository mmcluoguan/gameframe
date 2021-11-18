local rapidjson = require('rapidjson')
--模块名称定义
Module_name = {
    gm = 1, --gm模块
    test = 2, --测试模块
    email = 3, --邮件模块
}

--角色数据分类
Data_type = {
    base = 1, --基础数据
    goods = 2, --物品数据
}

--记录角色数据变化日志
--module_name 模块名称定义
--role_id 角色id
--data_type 角色数据分类
--data_desc 变化的数据表格
Record_Role_Data_Change = function (module_name,role_id,data_type,data_desc)
    local id = newid()
    local desc = rapidjson.encode(setmetatable(data_desc, {__jsontype='object'}))
    local insertdata = {
        cache_key = 'roledatachange_' ..  id .. "_" .. role_id,
        opertype = 2,
        fields = {
            { key = '_id', value = tostring(id),},
            { key = 'module_name', value = tostring(module_name),},
            { key = 'role_id', value = tostring(role_id),},
            { key = 'data_type', value = tostring(data_type),},
            { key = 'time', value = tostring(os.time()),},
            { key = 'desc', value = desc,},
        }
    }
    ConnectorMgr:dbConnector():send('insertdata_to_dbvisit_c',insertdata)
end