.start:
    inbuf word
    0branch .endline
    inbuf find
    dup
    0branch .number

    state @
    0branch .interpreting

    dup flag
    0branch .compiling

.interpreting:
    execute
    branch .start

.compiling:
    ,
    branch .start

.number:
    drop
    inbuf number

    state @
    0branch .start

    lit lit , ,
    branch .start

.endline:
    cr
    branch .start
