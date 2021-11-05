local timerMgr = {}

--计时器回调
--timer_id 计时器id
function onTimerOut(timer_id)
    --print('计时器回调',timer_id)
    timerMgr:execute(timer_id)
end

--绑定计时器
--millisecond 毫秒
--repeated 是否重复执行 默认true 值为false不需要unbind
--table 表变量,如果绑定的是全局函数,此设置为nil
--funname 函数名
--paras 调用函数参数列表
function timerMgr:bind(millisecond,repeated,table,funname,...)
    local tb = {
        millisecond = millisecond,
        table = table,
        funname = funname,
        paras = {...},
        repeated = repeated,
    }
    local timer_id = schedule_timer(millisecond,repeated)
    --print("绑定计时器",timer_id,self)
    self[timer_id] = tb
    return timer_id
end

--取消计时器
function timerMgr:unbind(timer_id)
    --print("取消计时器",timer_id)
    cancel_timer(timer_id)
    self[timer_id] = nil
end

function timerMgr:execute(timer_id)
    local tb = self[timer_id];
    if tb == nil then
        return
    end
    if tb.table == nil then
        tb.fun(table.unpack(tb.paras))
    else
        tb.table[tb.funname](table.unpack(tb.paras))
    end
    if tb.repeated == false then
        self[timer_id] = nil
    end
end

return timerMgr;
