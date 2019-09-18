# File server written in C 

### Instruction byte array

1. 1BYTE 	-> Instruction     (uint8_t)
2. 1BYTE 	-> Argument count  (uint8_t)
3. 4BYTE 	-> File size 	   (uint32_t)
4. 100BYTE 	-> First argument  (needs NUL termination)
5. 100BYTE 	-> Second argument (needs NUL termination)

### Instruction values

*sfn means server file name*

1.  PUSH = 0x00
2.  GET = 0x01
3.  RM = 0x02
4.  DIR = 0x03
5.  AUTH = 0x04
6.  GO = 0x05
7.  REV = 0x06
8.  PATH = 0x07
9.  MKFD = 0x08
10. RMFD = 0x09

### Response byte array

1. 4BYTE -> random key (uint32_t)
2. 1BYTE -> SUCCESS or FAIL (0xFF or 0x00)

## PUSH

Pushes file to the server in the root directory or any of its subdirectories.

1. Send instruction
2. Read response
3. Send file

## GET

Gets the file from the server in the root directory or any of its subdirectories.

1. Send instruction
2. Read file size
3. Read file
4. Read response

## RM

Removes a file from the server in the root directory or any of its subdirectories.

1. Send instruction
2. Read response

## DIR

Get all files and folder names in the root directory or any of its subdirectories. (seperator is '~')

1. Send instruction
2. Read data size
3. Read data
4. Read response

## AUTH

Logins to the server. No other instructions can be executed without loggin in.

1. Send instruction
2. Read response

## GO

Moves directory pointer to the selected folder.

1. Send instruction
2. Read response

## REV

Moves directory back.

1. Send instruction
2. Read response

## PATH

Returns the current path.

1. Send instruction
2. Read size (max 255)
3. Read data
4. Read response

## MKFD

Creates a folder in the current directory.

1. Send instruction
2. Read response

## RMFD

Removes a folder in the current directory.

1. Send instruction
2. Read response