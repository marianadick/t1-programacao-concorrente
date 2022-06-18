#include <stdlib.h>
#include "buffet.h"
#include "config.h"
#include "globals.h"
#include "chef.h"

int all_students_entered = FALSE;

void *buffet_run(void *arg)
{   
    buffet_t *self = (buffet_t*) arg;
    //queue_t* fila_fora = globals_get_queue();

    /*  O buffet funciona enquanto houver alunos na fila externa. */
    while (all_students_entered == FALSE)
    {
        msleep(500);
        /* Cada buffet possui: Arroz, Feijão, Acompanhamento, Proteína e Salada */
        /* Máximo de porções por bacia (40 unidades). */
        _log_buffet(self);
       /* Pode retirar este sleep quando implementar a solução! */
    }
    for (int i = 0; i < 5; i++) {
        pthread_mutex_destroy(&self->mut_left[i]);
        pthread_mutex_destroy(&self->mut_right[i]);
    }
    pthread_exit(NULL);
}

void buffet_init(buffet_t *self, int number_of_buffets)
{
    int i = 0, j = 0;
    globals_set_number_of_buffets(number_of_buffets);
    for (i = 0; i < number_of_buffets; i++)
    {
        /*A fila possui um ID*/
        self[i]._id = i;

        /* Inicia com 40 unidades de comida em cada bacia */
        for (j = 0; j < 5; j++) {
            self[i]._meal[j] = 40;
            pthread_mutex_init(&self[i].mut_left[j], NULL);
            pthread_mutex_init(&self[i].mut_right[j], NULL);
        }
        for(j= 0; j< 5; j++){
             /* A fila esquerda do buffet possui cinco posições. */
            self[i].queue_left[j] = 0;
            /* A fila esquerda do buffet possui cinco posições. */
            self[i].queue_right[j] = 0;
        }
        pthread_create(&self[i].thread, NULL, buffet_run, &self[i]);
    }
    sem_post(&chef_sync_buffes);
}


int buffet_queue_insert(buffet_t *self, student_t *student)
{
    /* Se o estudante vai para a fila esquerda */
    if (student->left_or_right == 'L') 
    {
        /* Verifica se a primeira posição está vaga */
        if (!self[student->_id_buffet].queue_left[0])
        {   
            /* Verifica se a primeira posição está vaga */
            pthread_mutex_lock(&self[student->_id_buffet].mut_left[0]);
            self[student->_id_buffet].queue_left[0] = student->_id;
            student->_buffet_position = 0;
            //descomentar para debbugar
            //printf("ESTUDANTE: %d entrou\n", student->_id);
            //fflush(stdout);
            return TRUE;
        }
        return FALSE;
    }
    else
    {   /* Se o estudante vai para a fila direita */
        if (!self[student->_id_buffet].queue_right[0])
        {
            /* Verifica se a primeira posição está vaga */
            pthread_mutex_lock(&self[student->_id_buffet].mut_right[0]);
            self[student->_id_buffet].queue_right[0] = student->_id;
            student->_buffet_position = 0;
            //descomentar para debbugar
            //printf("ESTUDANTE: %d entrou\n", student->_id);
            //fflush(stdout);
            return TRUE;
        }
        return FALSE;
    }
}


void buffet_next_step(buffet_t *self, student_t *student)
{
    /* Se estudante ainda precisa se servir de mais alguma coisa... */
    if (student->_buffet_position + 1 < 5)
    {    /* Está na fila esquerda? */
        if (student->left_or_right == 'L')
        {   /* Caminha para a posição seguinte da fila do buffet.*/
            int position = student->_buffet_position;
            //tenta lockar o mutex na próxima posição
            //garante que apenas um estudante esteja em cada posição
            pthread_mutex_lock(&self[student->_id_buffet].mut_left[position+1]);
            self[student->_id_buffet].queue_left[position] = 0;
            self[student->_id_buffet].queue_left[position + 1] = student->_id;
            student->_buffet_position = student->_buffet_position + 1;
            //libera o mutex da posição anterior
            pthread_mutex_unlock(&self[student->_id_buffet].mut_left[position]);
        }else /* Está na fila direita? */
        {   /* Caminha para a posição seguinte da fila do buffet.*/
            int position = student->_buffet_position;
            //tenta lockar o mutex na próxima posição
            //garante que apenas um estudante esteja em cada posição
            pthread_mutex_lock(&self[student->_id_buffet].mut_right[position+1]);
            self[student->_id_buffet].queue_right[position] = 0;
            self[student->_id_buffet].queue_right[position + 1] = student->_id;
            student->_buffet_position = student->_buffet_position + 1;
            //libera o mutex da posição anterior
            pthread_mutex_unlock(&self[student->_id_buffet].mut_right[position]);
        }
    } else {
        /* Está na última posição do buffet */
        if (student->left_or_right == 'L')
        {   /* Sai do buffet.*/
            int position = student->_buffet_position;
            self[student->_id_buffet].queue_left[position] = 0;
            //recebe posição -1 para indicar que esta fora do buffet
            student->_buffet_position = -1;
            //libera o mutex da ultima posição do buffet
            pthread_mutex_unlock(&self[student->_id_buffet].mut_left[4]);
        }else 
        {   /* Sai do buffet.*/
            int position = student->_buffet_position;
            self[student->_id_buffet].queue_right[position] = 0;
            //recebe posição -1 para indicar que esta fora do buffet
            student->_buffet_position = -1;
            //libera o mutex da ultima posição do buffet
            pthread_mutex_unlock(&self[student->_id_buffet].mut_right[4]);
        }
    }
}

/* --------------------------------------------------------- */
/* ATENÇÃO: Não será necessário modificar as funções abaixo! */
/* --------------------------------------------------------- */

void buffet_finalize(buffet_t *self, int number_of_buffets)
{
    /* Espera as threads se encerrarem...*/
    for (int i = 0; i < number_of_buffets; i++)
    {
        pthread_join(self[i].thread, NULL);
    }
    
    /*Libera a memória.*/
    free(self);
}


void _log_buffet(buffet_t *self)
{
    /* Prints do buffet */
    int *ids_left = self->queue_left; 
    int *ids_right = self->queue_right; 

    printf("\n\n\u250F\u2501 Queue left: [ %d %d %d %d %d ]\n", ids_left[0],ids_left[1],ids_left[2],ids_left[3],ids_left[4]);
    fflush(stdout);
    printf("\u2523\u2501 BUFFET %d = [RICE: %d/40 BEANS:%d/40 PLUS:%d/40 PROTEIN:%d/40 SALAD:%d/40]\n",
           self->_id, self->_meal[0], self->_meal[1], self->_meal[2], self->_meal[3], self->_meal[4]);
    fflush(stdout);
    printf("\u2517\u2501 Queue right: [ %d %d %d %d %d ]\n", ids_right[0],ids_right[1],ids_right[2],ids_right[3],ids_right[4]);
    fflush(stdout);
}