#include <stdlib.h>
#include <semaphore.h>
#include "worker_gate.h"
#include "globals.h"
#include "config.h"

buffet_t* buffet_livre = NULL;
char lado_livre = '0';
int id_prox = -1;
pthread_mutex_t mutex;
sem_t semaphore_a;

//Retorna o numero de alunos na fila de fora esperando para entrar
int worker_gate_look_queue(queue_t* fila_fora)
{
    return fila_fora->_length;
}

//retira o primeiro estudante da fila e libera a ''catraca'' pra ele
void worker_gate_remove_student(queue_t* fila_fora)
{
    student_t* proximo = queue_remove(fila_fora);
    int number = globals_get_students();
    globals_set_students(number - 1);
    id_prox = proximo->_id;
    proximo->_id_buffet = buffet_livre->_id;
    proximo->left_or_right = lado_livre;
    buffet_livre = NULL;
    lado_livre = '0';
    printf("%d number\n", number);
    fflush(stdout);
}

//fica procurando por todos os buffets até achar um livre,
//encontrado marca o lado e o qual o buffet
void worker_gate_look_buffet(buffet_t* buffet_array)
{
    int number_of_buffets = globals_get_number_of_buffets();
    while(TRUE) {
        //printf("entrou no look: %d", number_of_buffets);
        for (int i = 0; i < number_of_buffets; i++) {
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
    queue_t* fila_fora = globals_get_queue();
    buffet_t* buffet_array = globals_get_buffets();

    //number_students = *((int *)arg);
    number_students = globals_get_students();
    all_students_entered = number_students > 0 ? FALSE : TRUE;
    while (all_students_entered == FALSE)
    {
        if (number_students <= 0) {
            all_students_entered = TRUE;
            printf("foi embora");
            break;
        }
        //O mutex serve para evitar que o worker gate defina como livre um buffet que 
        // possuí um estudante a caminho porém ainda não tomou o lugar definido
        sem_wait(&semaphore_a);
        pthread_mutex_lock(&mutex);
        worker_gate_look_buffet(buffet_array);
        worker_gate_remove_student(fila_fora);
        printf("%d zzzzz\n", id_prox);
    }

    pthread_exit(NULL);
}

void worker_gate_init(worker_gate_t *self)
{
    int number_students = globals_get_students();
    pthread_mutex_init(&mutex, NULL);
    sem_init(&semaphore_a, 0, 0);
    pthread_create(&self->thread, NULL, worker_gate_run, &number_students);
}

void worker_gate_finalize(worker_gate_t *self)
{
    pthread_mutex_destroy(&mutex);
    sem_destroy(&semaphore_a);
    pthread_join(self->thread, NULL);
    free(self);
}

//Se o aluno é o próximo então direciona ele para o buffe livre e o lado livre.
void worker_gate_insert_queue_buffet(student_t *student)
{
    queue_t* fila_fora = globals_get_queue();
    queue_insert(fila_fora, student);
    sem_post(&semaphore_a);
    while (!(id_prox == student->_id)) {};    

    if (buffet_queue_insert(globals_get_buffets(), student) == FALSE) {
        printf("falhou \n");
    };
    pthread_mutex_unlock(&mutex);

}