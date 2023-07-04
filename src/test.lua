
local lu = require 'luaunit'
local wolfram = require 'wolfram'
local op = require 'operator'

local f = wolfram.wolfram (8675094)

for _, tbl in ipairs (f) do op.print_table (tbl); print '---' end