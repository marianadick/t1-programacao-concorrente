#include <stdlib.h>
#include <semaphore.h>
#include "worker_gate.h"
#include "globals.h"
#include "config.h"
#include "chef.h"
#include "table.h"

buffet_t* buffet_livre = NULL;
char lado_livre = '0';
pthread_mutex_t mutex;
pthread_mutex_t mut_table;
sem_t sem_sync_gate_student;
sem_t sem_empty_seats;
int count_entry = 0;

//Retorna o numero de alunos na fila de fora esperando para entrar
int worker_gate_look_queue(queue_t* fila_fora)
{
    return fila_fora->_length;
}

//Retira o primeiro estudante da fila e libera a ''catraca'' pra ele,
//aumenta o contador global count_entry
//define qual buffet o estudante vai através do id e o lado livre,
//e libera o semaforo do estudante
void worker_gate_remove_student(queue_t* fila_fora)
{
    student_t* proximo = queue_remove(fila_fora);
    count_entry++;
    proximo->_id_buffet = buffet_livre->_id;
    proximo->left_or_right = lado_livre;
    sem_post(&proximo->student_waiting);
    //Reseta o buffet livre pra null e o lado livre pra '0'
    buffet_livre = NULL;
    lado_livre = '0';
}

//fica procurando por todos os buffets até achar um livre,
//encontrado marca o lado e o qual o buffet
void worker_gate_look_buffet(buffet_t* buffet_array)
{
    int number_of_buffets = globals_get_number_of_buffets();
    while(TRUE) {
        //itera por todos os buffets
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
        //para de procurar quando existir um buffet_livre
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

    //Inicializa o semaforo que sincroniza o chef e os buffets
    sem_init(&chef_sync_buffes, 0, 0);

    number_students = globals_get_students();
    all_students_entered = number_students > 0 ? FALSE : TRUE;
    while (all_students_entered == FALSE)
    {
        if (number_students <= 0) {
            all_students_entered = TRUE;
            break;
        }
        //espera pelo primeiro estudante estar na fila pra começar
        sem_wait(&sem_sync_gate_student);

        //O mutex serve para evitar que o worker gate defina como livre um buffet que 
        // possuí um estudante a caminho porém ainda não tomou o lugar definido
        pthread_mutex_lock(&mutex);

        //Vai procurar um buffet livro até achar
        worker_gate_look_buffet(buffet_array);

        //É garantido que ele achou um buffet livre quando saiu da função look_buffet,
        //a verificação de lado livre é apenas uma garantia a mais
        if (lado_livre != '0'){
        //Direciona o primeiro estudante da fila pra o buffet
        worker_gate_remove_student(fila_fora);
        }
        //Cada estudante direcionado para o buffet in crementa o count_entry em um,
        //quando count_entry é igual ao total de estudantes significa que não há mais nenhum para entrar.
        //Portanto, o worker gate pode fechar as portas
        if (number_students == count_entry) {
            //Descomentar para debuggar
            //printf("WORKER_GATE: Fim de expediente.\n");
            //fflush(stdout);
            break;
        }
    }

    pthread_exit(NULL);
}

void worker_gate_init(worker_gate_t *self)
{
    int number_students = globals_get_students();
    int number_of_tables = globals_get_number_of_tables();
    int seats_per_table = globals_get_seats_per_table();
    //inicia o mutex que espera o estudante ir ao buffet 
    pthread_mutex_init(&mutex, NULL);
    //inicia o semaforo de sync com os estudantes
    sem_init(&sem_sync_gate_student, 0, 0);
    //inicia o semaforo que diz se tem acentos livres ou não
    sem_init(&sem_empty_seats, 0, number_of_tables*seats_per_table);
    //inicia o mutex de sentar nas mesas
    pthread_mutex_init(&mut_table, NULL);
    pthread_create(&self->thread, NULL, worker_gate_run, &number_students);
}

void worker_gate_finalize(worker_gate_t *self)
{
    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&mut_table);
    sem_destroy(&sem_sync_gate_student);
    sem_destroy(&sem_empty_seats);
    pthread_join(self->thread, NULL);
    free(self);
}

//Os alunos que chamam essa funçao. Eles entram na fila e esperam serem chamados.
//Cada aluno da um post no semaforo de sync do worker gate, permitindo mais um loop no worker gate
//Quando o remove student pega o primeiro da fila, ele libera o semaforo do estudante.
void worker_gate_insert_queue_buffet(student_t *student)
{
    queue_t* fila_fora = globals_get_queue();
    //Entra na fila externa
    queue_insert(fila_fora, student);
    //Libera o worker gate a fazer sua rotina
    sem_post(&sem_sync_gate_student);
    //Espera ser chamado pelo worker gate
    sem_wait(&student->student_waiting); 

    //insere o estudante no buffet
    //o if é apenas para debbug
    if (buffet_queue_insert(globals_get_buffets(), student) == FALSE) {
        printf("falhou");
    }
    //O estudante ja chegou ao buffet alvo, portanto libera o worker gate a procurar outro buffet livre
    pthread_mutex_unlock(&mutex);

}