/*
 * protocol.c
 *
 *  Created on: 8 août 2012
 *      Author: phsymo10
 */

#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#define OP_GET 0x00
#define OP_PUT 0x01
#define OP_RMV 0x02
#define OP_LIST 0x03

//TODO : Binary protocol

typedef struct reply_t{
	//TODO : multiple attribute
	int replied;
	char* name;
	char* value;

	unsigned char rc;
	char* message;

} reply_t;

typedef struct request_t{
	unsigned int id;
	unsigned char op;
	//TODO : multiple attribute
	char* name;
	char* value;
	reply_t reply;
} request_t;

int decode_request(request_t* request, char* req, int len);

void encode_reply(request_t* req, char* buff, int buff_len);

#endif /* PROTOCOL_H_ */
