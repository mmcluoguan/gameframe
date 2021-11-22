Get_Tablekey_Size = function(tab)
    local i = 0
    for _,_ in pairs(tab) do
        i = i + 1
    end
    return i
end

function Toboolean(arg)
    return not not arg
end

function StrtoBool(arg)
    assert(type(arg) == "string")
    if arg == "true" then
        return true
    else
        return false
    end
end

--距离当前0点秒数
function DistanceZeroTimeSeconds()
    local date = os.date("*t")
    date.hour = 23
    date.min = 23
    date.sec = 59
    return os.time(date) - os.time()
end