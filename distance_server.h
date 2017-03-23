/* 
 * File:   distance_server.h
 * Author: tomas
 *
 * Created on 19 February 2016, 02:34
 */

#ifndef DISTANCE_SERVER_H
#define	DISTANCE_SERVER_H

#ifdef	__cplusplus
extern "C" {
#endif


#include "jsonrpc-c.h"

cJSON * say_hello(jrpc_context * ctx, cJSON * params, cJSON *id);
cJSON * exit_server(jrpc_context * ctx, cJSON * params, cJSON *id);
cJSON * l_file(jrpc_context * ctx, cJSON * params, cJSON *id);

int main(void);

cJSON* return_distance(char* word);
cJSON* return_distance_vec(double* vec);

int load_file(char* file_name);

#ifdef	__cplusplus
}
#endif

#endif	/* DISTANCE_SERVER_H */

