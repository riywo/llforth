.start:
    inbuf word
    branch0 .end
    inbuf find
    execute
    branch .start

.end:
    bye
