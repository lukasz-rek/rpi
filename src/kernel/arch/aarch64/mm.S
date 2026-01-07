.globl memcpy
memcpy:
    cbz x2, memcpy_done      // if count == 0, return
    
memcpy_loop:
    ldrb w3, [x1], #1        // Load 1 byte from src, increment src
    strb w3, [x0], #1        // Store 1 byte to dest, increment dest
    subs x2, x2, #1          // Decrement count
    b.ne memcpy_loop         // Loop if count != 0
    
memcpy_done:
    ret

.globl memzero
memzero:
    cbz x1, memzero_done     // if count == 0, return
    
memzero_loop:
    strb wzr, [x0], #1       // Store zero byte, increment dest
    subs x1, x1, #1          // Decrement count
    b.ne memzero_loop        // Loop if count != 0
    
memzero_done:
    ret