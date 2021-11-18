get_tablekey_size = function(tab)
    local i = 0
    for _,_ in pairs(tab) do
        i = i + 1
    end
    return i
end

function toboolean(arg)
    return not not arg
end

function stobool(arg)
    assert(type(arg) == "string")
    if arg == "true" then
        return true
    else
        return false
    end
end