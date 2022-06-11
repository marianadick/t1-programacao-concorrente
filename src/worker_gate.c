#include <stdlib.h>

#include "worker_gate.h"
#include "globals.h"
#include "config.h"

buffet_t* buffet_livre = NULL;
char lado_livre = NULL;
int id_prox = NULL;
pthread_mutex_t mutex;

//Retorna o numero de alunos na fila de fora esperando para entrar
int worker_gate_look_queue(queue_t* fila_fora)
{
    return fila_fora->_length;
}

//retira o primeiro estudante da fila e libera a ''catraca'' pra ele
student_t* worker_gate_remove_student(queue_t* fila_fora)
{
    student_t* proximo = queue_remove(fila_fora);
    id_prox = proximo->_id;
    proximo->_id_buffet = buffet_livre->_id;
    proximo->left_or_right = lado_livre;
    buffet_livre = NULL;
    lado_livre = NULL;
}

//fica procurando por todos os buffets até achar um livre,
//encontrado marca o lado e o qual o buffet
void worker_gate_look_buffet(buffet_t* buffet_array)
{
    while(TRUE) {
    for (int i = 0; i < config.buffets; i++) {
        if (buffet_array[i].queue_left[0] == 0)   {
            buffet_livre = &buffet_array[i];
            lado_livre = 'L';
            break;
        } else if (buffet_array[i].queue_right[0] == 0) {
            buffet_livre = &buffet_array[i];
            lado_livre = 'R';
            break;
        }
    }
    if (buffet_livre != NULL){
        break;
    }
    }
}

void *worker_gate_run(void *arg)
{
    int all_students_entered;
    int number_students;
    int fila_de_fora;
    queue_t* fila_fora = globals_get_queue();
    buffet_t* buffet_array = globals_get_buffets();

    number_students = *((int *)arg);
    all_students_entered = number_students > 0 ? FALSE : TRUE;

    while (all_students_entered == FALSE)
    {
        fila_de_fora = worker_gate_look_queue(fila_fora);
        if (fila_de_fora <= 0) {
            all_students_entered = TRUE;
            break;
        }
        //O mutex serve para evitar que o worker gate defina como livre um buffet que 
        // possuí um estudante a caminho porém ainda não tomou o lugar definido
        pthread_mutex_lock(&mutex);
        worker_gate_look_buffet(buffet_array);
        worker_gate_remove_student(fila_fora);
    }

    pthread_exit(NULL);
}

void worker_gate_init(worker_gate_t *self)
{
    int number_students = globals_get_students();
    pthread_mutex_init(&mutex, NULL);
    pthread_create(&self->thread, NULL, worker_gate_run, &number_students);
}

void worker_gate_finalize(worker_gate_t *self)
{
    pthread_mutex_destroy(&mutex);
    pthread_join(self->thread, NULL);
    free(self);
}

//Se o aluno é o próximo então direciona ele para o buffe livre e o lado livre.
void worker_gate_insert_queue_buffet(student_t *student)
{
    while (!(id_prox == student->_id)) {};    

    if (buffet_queue_insert(globals_get_buffets(), student) == FALSE) {
        printf("falhou \n");
    };

    pthread_mutex_unlock(&mutex);

}