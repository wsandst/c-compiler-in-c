          global    main

          section   .text
main:                                       ; This is called by the C library startup code
          mov       rax, 2
          ret                               ; Return from main back into C library wrapper