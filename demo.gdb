# Print which `return -1` in check_free_list() fired, with no source edits.
#
# Needs a debug build (line numbers + variable names):
#   make debug
#
# Run with:
#   gdb -q -nx -batch -x demo.gdb --args env LD_PRELOAD=$PWD/libmalloc.so nm tiny
#
#   -nx     skip ~/.gdbinit (peda floods -batch output with Python warnings)
#   -batch  run the script and exit
#   --args  everything after is the program + its arguments

# The .so is not loaded yet when gdb starts, so breakpoints in it are unresolved
# until the dynamic loader maps it. Without this, -batch mode just errors out.
set breakpoint pending on
set pagination off
set confirm off

break check_heap.c:46
commands
  silent
  printf "  >>> tags not equal\n"
  continue
end

break check_heap.c:43
commands
  silent
  printf "  >>> footer invalid\n"
  continue
end

break check_heap.c:39
commands
  silent
  printf "  >>> size invalid\n"
  continue
end

break check_heap.c:35
commands
  silent
  printf "  >>> header invalid\n"
  continue
end

break check_free_list.c:17
commands
  silent
  printf "  >>> invalid free block\n"
  continue
end

break check_free_list.c:20
commands
  silent
  printf "  >>> free block allocated\n"
  continue
end

break check_free_list.c:58
commands
  silent
  printf " detect cycle free list\n"
  continue
end

break check_free_list.c:62
commands
  silent
  printf "  >>> free block invalid in main loop\n"
  continue
end

run

# Notes:
#   silent    suppresses gdb's own "Breakpoint 1, ..." banner
#   continue  makes the breakpoint non-stop, so the run produces a pure log
#   printf    same format specifiers as C, e.g.
#               printf "size=%lu ptr=%p\n", size, ptr
#             any variable in scope at that line can be printed
#
# Other edit-free alternatives:
#   finish        prints "Value returned is $1 = -1" when stopped in a function
#   display expr  re-prints expr at every stop
#   watch expr    stops when the value changes
#   info locals   dumps every local at once
