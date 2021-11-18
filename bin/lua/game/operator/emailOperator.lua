--邮件操作
local dynamicData = require("lua/game/data/dynamicData")
local emailOperator = {}

--给玩家添加一封新邮件
function emailOperator:add_one_email(role,email_id)
    local emaildata = dynamicData.emaildata:new()
    emaildata.id = email_id
    role.emails[email_id] = emaildata

    --更新数据库
    local emaildata = {
        cache_key = 'email_' .. email_id .. "_" .. role.id,
        opertype = 0,
        fields = {
            { key = '_id', value = tostring(email_id),},
            { key = 'roleid', value = tostring(role.id),},
            { key = 'is_read', value = tostring(emaildata.is_read),},
            { key = 'is_receive', value = tostring(emaildata.is_receive),},
        }
    }
    ConnectorMgr:dbConnector():send('insertdata_to_dbvisit_c',emaildata)

    --发送新邮件通知
    role:send('email_new_client_gate_g',{
        id = email_id
    })
end

--从系统中收集离线邮件到角色邮件列表
function emailOperator:collect_unline_email(role)
    for key, value in pairs(EmailSystem.emails) do
        if value.time > role.unline_time and role.emails[key] == nil then
            local email = dynamicData.emaildata:new()
            email.id = key
            role.emails[key] = email

            --更新数据库
            local emaildata = {
                cache_key = 'email_' .. key .. "_" .. role.id,
                opertype = 0,
                fields = {
                    { key = '_id', value = tostring(key),},
                    { key = 'roleid', value = tostring(role.id),},
                    { key = 'is_read', value = tostring(email.is_read),},
                    { key = 'is_receive', value = tostring(email.is_receive),},
                }
            }
            ConnectorMgr:dbConnector():send('insertdata_to_dbvisit_c',emaildata)
        end
    end    
end

return emailOperator