#ifndef PROTOCOL_H
#define PROTOCOL_H

/* 
 * Here we define status codes for messages between Client and Server
 */ 

enum {C_OK, C_ERROR, C_PUT_SENDING, C_PUT_FINISHED, C_EXIT, C_CMD, C_INPUT, C_GET};
enum {S_OK, S_ERROR, S_GET_SENDING, S_GET_FINISHED, S_PRG_RUNNING, S_PRG_TERM, S_WALL, S_OUTPUT};

#endif
