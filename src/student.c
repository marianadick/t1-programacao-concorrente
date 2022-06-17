#include <time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

#include "student.h"
#include "config.h"
#include "worker_gate.h"
#include "globals.h"
#include "table.h"

pthread_mutex_t mut_table;
sem_t sem_empty_seats;

void* student_run(void *arg)
{
    student_t *self = (student_t*) arg;
    sem_init(&self->student_waiting, 0, 0);
    table_t *tables  = globals_get_table();
    worker_gate_insert_queue_buffet(self);
    student_serve(self);
    student_seat(self, tables);
    student_leave(self, tables);
    sem_destroy(&self->student_waiting);
    pthread_exit(NULL);
};

void student_seat(student_t *self, table_t *table)
{
    int number_of_tables = globals_get_number_of_tables();

    sem_wait(&sem_empty_seats);
    pthread_mutex_lock(&mut_table);
    for (int i = 0; i < number_of_tables; i++) {
        //pthread_mutex_trylock(&mut_table);
        if (table[i]._empty_seats > 0) {
            self->_id_table = table[i]._id;
            table[i]._empty_seats--;
            printf("aluno %d sentou na mesa %d \n", self->_id, table[i]._id);
            fflush(stdout);
        //pthread_mutex_unlock(&mut_table);
            break;
        }
    }
    pthread_mutex_unlock(&mut_table);
}

void student_serve(student_t *self)
{
    buffet_t* buffet = globals_get_buffets();
    while (TRUE) {
        msleep(500);
        while(buffet[self->_id_buffet]._meal[self->_buffet_position] == 0) {};
        if (self->_wishes[self->_buffet_position] == 1 ) {
            buffet[self->_id_buffet]._meal[self->_buffet_position]--;
        }
        buffet_next_step(buffet, self);
        if (self->_buffet_position == -1) {
            break;
        }
    }
}

void student_leave(student_t *self, table_t *table)
{
    // mutex no decremento (so sai 1)
    msleep(1000);
    pthread_mutex_lock(&mut_table);
    printf("aluno %d saiu da mesa %d e foi embora\n", self->_id, self->_id_table);
    fflush(stdout);
    table[self->_id_table]._empty_seats++;
    pthread_mutex_unlock(&mut_table);
    sem_post(&sem_empty_seats);    
}

/* --------------------------------------------------------- */
/* ATENÇÃO: Não será necessário modificar as funções abaixo! */
/* --------------------------------------------------------- */

student_t *student_init()
{
    student_t *student = malloc(sizeof(student_t));
    student->_id = rand() % 1000;
    student->_buffet_position = -1;
    int none = TRUE;
    for (int j = 0; j <= 4; j++)
    {
        student->_wishes[j] = _student_choice();
        if(student->_wishes[j] == 1) none = FALSE;
    }

    if(none == FALSE){
        /* O estudante só deseja proteína */
        student->_wishes[3] = 1;
    }
    
    return student;
};

void student_finalize(student_t *self){
    free(self);
};


pthread_t students_come_to_lunch(int number_students)
{
    pthread_t lets_go;
    pthread_create(&lets_go, NULL, _all_they_come, &number_students);
    return lets_go;
}

/**
 * @brief Função (privada) que inicializa as threads dos alunos.
 * 
 * @param arg 
 * @return void* 
 */
void* _all_they_come(void *arg)
{
    int number_students = 100;
    // *((int *)arg);
    student_t *students[100];

    for (int i = 0; i < number_students; i++)
    {
        students[i] = student_init();                                               /* Estudante é iniciado, recebe um ID e escolhe o que vai comer*/
    }

    for (int i = 0; i < number_students; i++)
    {
        pthread_create(&students[i]->thread, NULL, student_run, students[i]);       /*  Cria as threads  */
    }

    for (int i = 0; i < number_students; i++)
    {
        pthread_join(students[i]->thread, NULL);                                    /*  Aguarda o término das threads   */
    }

    for (int i = 0; i < number_students; i++)
    {
        student_finalize(students[i]);                                              /*  Libera a memória de cada estudante  */
    }

    pthread_exit(NULL);
}

/**
 * @brief Função que retorna as escolhas dos alunos, aleatoriamente (50% para cada opção)
 *        retornando 1 (escolhido) 0 (não escolhido). É possível que um aluno não goste de nenhuma opção
 *         de comida. Nesse caso, considere que ele ainda passa pela fila, como todos aqueles que vão comer.
 * @return int 
 */
int _student_choice()
{
    float prob = (float)rand() / RAND_MAX;
    return prob > 0.51 ? 1 : 0;
}