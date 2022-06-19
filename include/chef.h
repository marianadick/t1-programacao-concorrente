#ifndef __chef_H__
#define __chef_H__
#include <semaphore.h>
#include <pthread.h>
#include <buffet.h>

typedef struct chef
{
    pthread_t thread;
} chef_t;

/* ALTERADO */
/* Semáforo de sincronização do Chef */
extern sem_t chef_sync_buffes;


/**
* @brief Inicializa a thread do chef.
* 
* @param  none
*/
extern void chef_init(chef_t *self);                           

/**
 * @brief Finaliza a thread do chef.
 * 
 * @param self 
 */
extern void chef_finalize(chef_t *self);

/**
* @brief Função de thread do chef.
* 
* @param  none
*/
extern void* chef_run();
    

/**
* @brief Chefe coloca comida no buffet.
* 
* @param buffet
* @param meal
* @param buffet_array
*/
extern void chef_put_food(int buffet, int meal, buffet_t* buffet_array);

/**
* @brief Chefe checa comida no buffet.
* 
* @param  none
*/                  
extern int chef_check_food();


#endif