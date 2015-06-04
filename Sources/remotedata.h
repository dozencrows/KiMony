/*
 * remotedata.h
 *
 *  Created on: 4 Jun 2015
 *      Author: ntuckett
 */

#ifndef REMOTEDATA_H_
#define REMOTEDATA_H_

typedef struct _RemoteDataHeader {
	uint32_t	homeActivityOffset;
	int			deviceCount;
	uint32_t	devicesOffset;
} RemoteDataHeader;

#endif /* REMOTEDATA_H_ */
