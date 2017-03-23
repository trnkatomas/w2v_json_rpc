//  Copyright 2013 Google Inc. All Rights Reserved.
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <malloc.h>
#include <stdio.h>

#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include "distance_server.h"

#define PORT 1234  // the port users will be connecting to
#define N 40

struct jrpc_server my_server;
const long long max_size = 2000; // max length of strings
// static const long long N = 40; // number of closest words that will be shown
const long long max_w = 50; // max length of vocabulary entries
char *vocab;
float *M;
char *bestw[N];
long long words, size, a, b, bi[100];

cJSON * say_hello(jrpc_context * ctx, cJSON * params, cJSON *id) {
	return cJSON_CreateString("Hello!");
}

cJSON * exit_server(jrpc_context * ctx, cJSON * params, cJSON *id) {
	jrpc_server_stop(&my_server);
	return cJSON_CreateString("Bye!");
}

cJSON * l_file(jrpc_context * ctx, cJSON * params, cJSON *id) {
        printf("Load file executed");
        cJSON *json = params->child;
        int out;
        if (json->type == cJSON_String){
            char* filename = json->valuestring;
            out = load_file(filename);
        }else{
            out = -1;
        }       
        return cJSON_CreateNumber(out);    	
}

cJSON * distance(jrpc_context * ctx, cJSON * params, cJSON *id) {
        cJSON *json = params->child;
        int out;
        if (json->type == cJSON_String){
            char* word = json->valuestring;
            return return_distance(word);
        }else{
            out = -1;
        }       
        return cJSON_CreateNumber(out);    	
}

cJSON * distance_vec(jrpc_context * ctx, cJSON * params, cJSON *id) {
        cJSON *json = params->child;
        int out;
        if (json->type == cJSON_Array){
            double* v = (double*) malloc(sizeof(double) * size);
            for (int i=0;i<size;i++){
                v[i] = cJSON_GetArrayItem(json, i)->valuedouble;
            }            
            return return_distance_vec(v);
        }else{
            out = -1;
        }       
        return cJSON_CreateNumber(out);    	
}

int main(void) {
	jrpc_server_init(&my_server, PORT);
	jrpc_register_procedure(&my_server, say_hello, "sayHello", NULL );
	jrpc_register_procedure(&my_server, exit_server, "exit", NULL );
        jrpc_register_procedure(&my_server, l_file, "loadFile", NULL );
        jrpc_register_procedure(&my_server, distance, "distance", NULL );
        jrpc_register_procedure(&my_server, distance_vec, "distanceVec", NULL );
	jrpc_server_run(&my_server);
	jrpc_server_destroy(&my_server);
	return 0;
}

cJSON* return_distance_vec(double* vec){
    cJSON *response = cJSON_CreateObject();
    char st1[max_size];
    char st[100][max_size];
    float dist, len, bestd[N];//, vec[max_size];
    long long a, b, c, d, cn;
    for (a = 0; a < N; a++) bestd[a] = 0;
    for (a = 0; a < N; a++) bestw[a][0] = 0;
    a = 0;;
    len = 0;
    for (a = 0; a < size; a++) len += vec[a] * vec[a];
    len = sqrt(len);
    for (a = 0; a < size; a++) vec[a] /= len;
    for (a = 0; a < N; a++) bestd[a] = -1;
    for (a = 0; a < N; a++) bestw[a][0] = 0;
    for (c = 0; c < words; c++) {
        a = 0;
        for (b = 0; b < cn; b++) if (bi[b] == c) a = 1;
        if (a == 1) continue;
        dist = 0;
        for (a = 0; a < size; a++) dist += vec[a] * M[a + c * size];
        for (a = 0; a < N; a++) {
            if (dist > bestd[a]) {
                for (d = N - 1; d > a; d--) {
                    bestd[d] = bestd[d - 1];
                    strcpy(bestw[d], bestw[d - 1]);
                }
                bestd[a] = dist;
                strcpy(bestw[a], &vocab[c * max_w]);
                break;
            }
        }
    }
    cJSON *wrds = cJSON_CreateArray();
    for (a = 0; a < N; a++){
        printf("%50s\t\t%f\n", bestw[a], bestd[a]);
        cJSON *w = cJSON_CreateObject();
        cJSON_AddStringToObject(w,"word",bestw[a]);
        cJSON_AddNumberToObject(w,"distance",bestd[a]);
        cJSON_AddItemToArray(wrds,w);
    }
    cJSON_AddItemToObject(response,"results",wrds);
    return response;
}

cJSON* return_distance(char* word){
    cJSON *response = cJSON_CreateObject();
    char st1[max_size];
    char st[100][max_size];
    float dist, len, bestd[N], vec[max_size];
    long long a, b, c, d, cn;
      for (a = 0; a < N; a++) bestd[a] = 0;
        for (a = 0; a < N; a++) bestw[a][0] = 0;
        a = 0;
        printf("%s\n", word);
        cJSON_AddStringToObject(response,"word",word);
        strcpy(st1, word);            
        cn = 0;
        b = 0;
        c = 0;
        while (1) {
            st[cn][b] = st1[c];
            b++;
            c++;
            st[cn][b] = 0;
            if (st1[c] == 0) break;
            if (st1[c] == ' ') {
                cn++;
                b = 0;
                c++;
            }
        }
        cn++;
        for (a = 0; a < cn; a++) {
            for (b = 0; b < words; b++) if (!strcmp(&vocab[b * max_w], st[a])) break;
            if (b == words) b = -1;
            bi[a] = b;
            printf("%d",sizeof(vocab));
            printf("\nWord: %s  Position in vocabulary: %lld\n", st[a], bi[a]);
            if (b == -1) {
                printf("Out of dictionary word!\n");
                break;
            }
        }
        if (b == -1) return cJSON_CreateString("-1");
        
        
        printf("\n                                              Word       Cosine distance\n------------------------------------------------------------------------\n");
        for (a = 0; a < size; a++) vec[a] = 0;
        for (b = 0; b < cn; b++) {
            if (bi[b] == -1) continue;
            for (a = 0; a < size; a++) vec[a] += M[a + bi[b] * size];
        }
        
        
        
        len = 0;
        for (a = 0; a < size; a++) len += vec[a] * vec[a];
        len = sqrt(len);
        for (a = 0; a < size; a++) vec[a] /= len;
        for (a = 0; a < N; a++) bestd[a] = -1;
        for (a = 0; a < N; a++) bestw[a][0] = 0;
        for (c = 0; c < words; c++) {
            a = 0;
            for (b = 0; b < cn; b++) if (bi[b] == c) a = 1;
            if (a == 1) continue;
            dist = 0;
            for (a = 0; a < size; a++) dist += vec[a] * M[a + c * size];
            for (a = 0; a < N; a++) {
                if (dist > bestd[a]) {
                    for (d = N - 1; d > a; d--) {
                        bestd[d] = bestd[d - 1];
                        strcpy(bestw[d], bestw[d - 1]);
                    }
                    bestd[a] = dist;
                    strcpy(bestw[a], &vocab[c * max_w]);
                    break;
                }
            }
        }
        cJSON *wrds = cJSON_CreateArray();
        for (a = 0; a < N; a++){
            printf("%50s\t\t%f\n", bestw[a], bestd[a]);
            cJSON *w = cJSON_CreateObject();
            cJSON_AddStringToObject(w,"word",bestw[a]);
            cJSON_AddNumberToObject(w,"distance",bestd[a]);
            cJSON_AddItemToArray(wrds,w);
        }
    cJSON_AddItemToObject(response,"results",wrds);
    return response;
}

int load_file(char* file_name) {
    if (words > 0) return 0;
    FILE *f;
    f = fopen(file_name, "rb");    
    float len;
    
    if (f == NULL) {
        printf("Input file: %s not found\n", file_name);
        return -1;
    }
    fscanf(f, "%lld", &words);
    fscanf(f, "%lld", &size);
    vocab = (char *) malloc((long long) words * max_w * sizeof (char));
    for (a = 0; a < N; a++) bestw[a] = (char *) malloc(max_size * sizeof (char));
    M = (float *) malloc((long long) words * (long long) size * sizeof (float));
    if (M == NULL) {
        printf("Cannot allocate memory: %lld MB    %lld  %lld\n", (long long) words * size * sizeof (float) / 1048576, words, size);
        return -1;
    }
    for (b = 0; b < words; b++) {
        a = 0;
        while (1) {
            vocab[b * max_w + a] = fgetc(f);
            if (feof(f) || (vocab[b * max_w + a] == ' ')) break;
            if ((a < max_w) && (vocab[b * max_w + a] != '\n')) a++;
        }
        vocab[b * max_w + a] = 0;
        for (a = 0; a < size; a++) fread(&M[a + b * size], sizeof (float), 1, f);
        len = 0;
        for (a = 0; a < size; a++) len += M[a + b * size] * M[a + b * size];
        len = sqrt(len);
        for (a = 0; a < size; a++) M[a + b * size] /= len;
    }
    fclose(f);
}