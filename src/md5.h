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
 * md5.h
 *
 *  Created on: 1 janv. 2013
 *      Author: Pierre-Henri Symoneaux
 */

#ifndef MD5_H_
#define MD5_H_

#define MD5_DIGEST_LENGTH 32


unsigned *md5(const char *msg, int mlen);

void md5_to_str(unsigned *d, char* str);

#endif /* MD5_H_ */
