## File server written in C 

## Instruction byte array

1. 1BYTE 	-> Instruction     (uint8_t)
2. 1BYTE 	-> Argument count  (uint8_t)
3. 4BYTE 	-> File size 	   (uint32_t)
4. 100BYTE 	-> First argument  (needs NUL termination)
5. 100BYTE 	-> Second argument (needs NUL termination)

## Instruction values

*fn means file name*

1. PUSH [fn]			= 1 << 0
   -push a file to the server

2. GET  [fn]			= 1 << 1
   -get a file from the server

3. REM  [fn] 			= 1 << 2
   -removes file

4. UP	[server_fn] [local_fn]  = 1 << 3
   -update a file on the server

5. DIR 				= 1 << 4
   -names of all the files in your server folder

*GET and DIR first send a 4BYTE message that conaints the file size in little endian
type uint32_t*
