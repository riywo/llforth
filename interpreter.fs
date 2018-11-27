.start:
    inbuf word
    branch0 .endline
    inbuf find
    dup
    branch0 .number

    state @
    branch0 .interpreting

    dup flag
    branch0 .compiling

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
    branch0 .start

    lit lit , ,
    branch .start

.endline:
    branch .start
