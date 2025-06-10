octal
177000 constant sd11-command
177000 constant sd11-status
177002 constant sd11-data
000000 constant cmd-init
040000 constant cmd-read
100000 constand cmd-write
decimal
: consume-block ( -- )
  sd11-status @ 0= if
    512 0 do sd11-data @ drop loop
  else
    ." cannot read "
  then ;
: .sd11-status ( -- )
  sd11-status @ . ;
: test-boot ( -- )
  cmd-init sd11-command !
  cmd-read sd11-command !
  7 0 do consume-block loop ;
