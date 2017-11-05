divert(-1)
#
# 2017/05/19 Josef Kubin
#
# generates an index to tracepoint(0b...)
# Copyright (C) 2017 Josef Kubin
#
# This file is part of opentracer.
#
# opentracer is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# opentracer is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

# prints help and exit
ifdef(`TRACE_POINT_WIDTH', `', `errprint(__file__:__line__`: error: m4 -DTRACE_POINT_WIDTH=2 -DTARGET_TIMER_WIDTH=1 -DTRACE_POINT_FILE=counter.txt -DTRACE_FILE=file.c tracepoint.m4 file.pnt > file.c
')m4exit(0)')

# init several macros
define(`TRACE_POINT_COUNTER', 0)
define(`TRACE_POINT_BITWIDTH', ifdef(`TRACE_TRIVIAL', `eval(TRACE_POINT_WIDTH * 8 + 1)', `eval(TRACE_POINT_WIDTH * 8 - 1)'))
define(`TRACE_POINT_MAX', eval(2 ** TRACE_POINT_BITWIDTH - 1))

# if a counter file with exists, set sysval to 0
syscmd(`test -f 'TRACE_POINT_FILE)

# if a counter file exists, load last value
ifelse(sysval, 0, `esyscmd(`cat 'TRACE_POINT_FILE)')

# the following macro generates unique number to tracepoint(0b...)
#
# what match:
# tracepoint(.*)
#
# what ignore:
# tracepoint
# tracepoint (unsigned char pnt)
define(`tracepoint', `ifelse(`$#', `0', ``$0'', ``$0(0b'eval(TRACE_POINT_COUNTER, 2, TRACE_POINT_BITWIDTH))ifelse(TRACE_POINT_COUNTER, TRACE_POINT_MAX, `errprint(TRACE_FILE:__line__`: warning: tracepoint(0b'eval(TRACE_POINT_COUNTER, 2, TRACE_POINT_BITWIDTH)`) ---> tracepoint(0b'eval(0, 2, TRACE_POINT_BITWIDTH)`)'
)define(`TRACE_POINT_COUNTER', -1)')define(`TRACE_POINT_COUNTER', incr(TRACE_POINT_COUNTER))')')

# at the end save the last counter value to the counter file
m4wrap(`errprint(`tracepoint('TRACE_POINT_COUNTER`)'
)syscmd(echo "`define(\`TRACE_POINT_COUNTER', 'TRACE_POINT_COUNTER`)'" > TRACE_POINT_FILE)')

divert(0)dnl
