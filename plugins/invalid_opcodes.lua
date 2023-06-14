-- This file is part of Luad (https://github.com/imring/Luad).
-- Licensed under the GNU General Public License v3.0.
-- Copyright (C) 2023 Vitaliy Vorobets

local function message(fmt, ...) print(fmt:format(...)) end

local function has_b_field(mode)
    return bit.band(bit.rshift(mode, 3), luajit.bcmode.MAX) ~= luajit.bcmode.none
end

local function check_proto(opcodes, proto)
    local result = {}

    local function check_field(i, field, mode)
        -- dst / base / var / rbase
        if mode == luajit.bcmode.dst or mode == luajit.bcmode.base
        or mode == luajit.bcmode.var or mode == luajit.bcmode.rbase then
            if field > proto.framesize then
                return false
            end
        end

        -- uv
        if mode == luajit.bcmode.uv then
            if field > #proto:uv_values() then
                return false
            end
        end

        -- lit / lits
        -- soon...

        -- pri
        if mode == luajit.bcmode.pri then
            if field ~= 0 and field ~= 1 and field ~= 2 then
                return false
            end
        end

        -- num
        if mode == luajit.bcmode.num then
            if field > #proto:knum_values() then
                return false
            end
        end

        -- str / tab / func / cdata
        if mode == luajit.bcmode.str or mode == luajit.bcmode.tab
        or mode == luajit.bcmode.func or mode == luajit.bcmode.cdata then
            if field > #proto:kgc_values() then
                return false
            end
        end

        -- jump
        if mode == luajit.bcmode.jump then
            local label = field + i + 1 - 0x8000
            if label > #proto:instructions() then
                return false
            end
        end

        return true
    end

    local function check_instruction(i, ins)
        -- invalid opcode
        local info = opcodes[ins.opcode]
        if info == nil then
            table.insert(result, i)
            return
        end

        local amode = bit.band(info.mode, 7)
        local bmode = bit.band(bit.rshift(info.mode, 3), luajit.bcmode.MAX)
        local cdmode = bit.band(bit.rshift(info.mode, 7), luajit.bcmode.MAX)
        local valid = check_field(i, ins.a, amode)
        if valid then
            if has_b_field(info.mode) then
                valid = check_field(i, ins.b, bmode) and check_field(i, ins.c, cdmode)
            else
                valid = check_field(i, ins.d, cdmode)
            end
        end

        if not valid then
            table.insert(result, i)
        end
    end

    for i, k in ipairs(proto:instructions()) do
        check_instruction(i, k)
    end
    return result
end

local function find_instructions(file, proto_id, instructions)
    local divs = file:dump_info().divs
    local proto = divs:additional()[proto_id + 2] -- +2 to skip header info
    local result = {}

    if #instructions == 0 then
        return result
    end

    local ins_id = 1
    for i, k in ipairs(proto:additional()) do
        if k.header == '.ins' then
            local i, skip = 1, 0
            local lines = k:lines()
            while i ~= #lines do
                if #lines[i].text == 0 then
                    skip = skip + 2
                    i = i + 2
                elseif lines[i].text:sub(1, 5) == 'label' then
                    skip = skip + 1
                    i = i + 1
                end

                if i - skip == instructions[ins_id] then
                    ins_id = ins_id + 1
                    local line = lines[i]
                    table.insert(result, { line.from, line.to })

                    if ins_id - 1 == #instructions then
                        goto label_result
                    end
                end

                i = i + 1
            end
            break
        end
    end

::label_result::
    return result
end

function on_open_file(file)
    local info = file:dump_info().info
    local count = 0

    if info:compiler() ~= compilers.luajit then
        message('plugin supports luajit only')
        return
    end
    local opcodes = info.version == 1 and luajit.v1.opcodes
                 or info.version == 2 and luajit.v2.opcodes
                 or nil

    message('finding invalid opcodes...')
    for i, k in ipairs(info:protos()) do
        local result = find_instructions(file, i, check_proto(opcodes, k))
        for l, p in ipairs(result) do
            highlight(p[1], p[2], 0xFFFF00)
        end
        count = count + #result
    end

    if count > 0 then
        message('found %d invalid opcodes', count)
        message('they are highlighted in yellow')
    else
        message('not found invalid opcodes')
    end
end