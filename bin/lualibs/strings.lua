b2string = function(value)
    if type(value) ~= 'string' then
        return tostring(value)
    end

    local s = ''
    for i = 1, string.len(value) do
        b = string.byte(value, i)
        if b < 16 then
            s = s .. string.format("0%X ", b)
        else
            s = s .. string.format("%X ", b)
        end
    end
    return s
end

t2string = function(value, layer)
    if type(value) ~= 'table' then
        return tostring(value)
    end
    layer = layer or 1
    local s = '{\n'
    for k, v in pairs(value) do
        s = s..string.rep('    ', layer)
        if type(k) == 'string' then
            s = s..k
        else
            s = s..'['
            s = s..tostring(k)
            s = s..']'
        end
        s = s..' = '
        if type(v) == 'table' then
            s = s..t2string(v, layer + 1)
        elseif type(v) == 'string' then
            s = s..string.format("%q",v)
        else
            s = s..tostring(v)
        end
        s = s..',\n'
    end
    s = s..string.rep('    ', layer - 1)
    s = s..'}'
    return s
end

split = function (input, delimiter)
    if type(delimiter) ~= "string" or #delimiter <= 0 then
        return
    end
    local start = 1
    local arr = {}
    while true do
        local pos = string.find(input, delimiter, start, true)
        if not pos then
            break
        end
        table.insert (arr, string.sub (input, start, pos - 1))
        start = pos + string.len (delimiter)
    end
    table.insert (arr, string.sub (input, start))
    return arr
end