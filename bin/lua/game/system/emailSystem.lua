--邮件系统
local email = require("lua/game/data/emailMatedata")
local dynamicData = require("lua/game/data/dynamicData")
local rapidjson = require('rapidjson')

local emailSystem = {
    emails = {},
    is_load = false, 
}

--加载系统邮件列表
function emailSystem:load()
    if self.is_load == false then
        local emaildata = {
            tag = "loadsysemail",
            condition = "sysemail_*_*",
            sort = "time desc",
            limit = 100,
            opertype = 2,
            fields = {
                {key = '_id', value = '',},
                {key = 'title', value = '',},
                {key = 'info', value = '',},
                {key = 'type', value = '',},
                {key = 'time', value = '',},
                {key = 'annex', value = '',},
            },
        }
        ConnectorMgr:dbConnector():send('loaddata_more_from_dbvisit_c',emaildata)
        self.is_load = true
    end
end

--加载系统邮件列表完成
function emailSystem:loaded(objs)
    for i = 1, #objs do
        self.emails[i] = email.emaildata:new()
        local emailid = nil
        for j = 1, #objs[i].fields do
            local key = objs[i].fields[j].key
            local value = objs[i].fields[j].value
            if key == '_id' then
                self.emails[i].id = tonumber(value)
                emailid = self.emails[i].id
            elseif key == 'type' then
                self.emails[i].type = tonumber(value)
            elseif key == 'time' then
                self.emails[i].time = tonumber(value)
            elseif key == 'annex' then
                self.emails[i].annex = rapidjson.decode(value)
            else
                self.emails[i][key] = value
            end
        end
        self.emails[emailid] = self.emails[i]
        self.emails[i] = nil
    end
    log('系统邮件列表加载完成 len:',get_tablekey_size(self.emails))
end

--添加一封新邮件
function emailSystem:add_new(emailMatedata)
    local data = email.emaildata:new()
    data.id = emailMatedata.id
    data.title = emailMatedata.title
    data.info = emailMatedata.info
    data.type = emailMatedata.type
    data.time = emailMatedata.time
    data.annex = email.annexdata:new();
    data.annex.gold = emailMatedata.annex.gold
    data.annex.diamond = emailMatedata.annex.diamond
    for key, value in ipairs(emailMatedata.annex.goods) do
        local goods = dynamicData.goodsdata:new()
        goods.cfgid = value.cfgid
        goods.num = value.num
        table.insert(data.annex.goods, goods)
    end
    self.emails[data.id] = data
end

return emailSystem;