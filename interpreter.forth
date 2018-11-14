.start:
    inbuf word
    branch0 .end
    inbuf find
    dup
    branch0 .number
    execute
    branch .start

.number:
    drop
    inbuf number
    branch .start

.end:
    bye
