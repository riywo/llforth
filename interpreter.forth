.start:
    inbuf word
    branch0 .end
    inbuf find
    dup
    branch0 .number
    execute
    branch .start

.number:
\    inbuf number
    branch .start

.end:
    bye
