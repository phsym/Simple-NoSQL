/*
* Simple-NoSQL
* Copyright (C) 2012 Pierre-Henri Symoneaux
* 
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 US
*/

/*
 * protocol.c
 *
 *  Created on: 8 août 2012
 *      Author: Pierre-Henri Symoneaux
 */

#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#define OP_GET 0x00
#define OP_PUT 0x01
#define OP_RMV 0x02
#define OP_LIST 0x03
#define OP_SET 0x04
#define OP_MD5 0x05

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
