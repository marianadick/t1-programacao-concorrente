#include <stdlib.h>
#include <semaphore.h>
#include "chef.h"
#include "config.h"
#include "globals.h"

sem_t chef_sync_buffes;

/* Chef checa o buffet e adiciona comida enquanto tem estudantes se servindo,
quando não há mais estudantes no buffet ou na fila, o chef encerra */
void *chef_run()
{
    /* Este semáforo sincroniza o chef com os buffets, impedindo
    que ele percorra os buffets antes da inicialização dos mesmos */
    sem_wait(&chef_sync_buffes);

    while (TRUE)
    {
        // Variáveis de controle para o fim do expediente do chef
        int there_is_students = globals_get_there_is_students();
        int outside_students = globals_get_students();

        // Chef checa se há comida no buffet e estudantes se servindo
        globals_set_there_is_students(chef_check_food());

        // Caso a fila e o buffet estiverem sem estudantes, o chef encerra
        if (!there_is_students && outside_students == 0) {
            printf("CHEF: Fim do expediente! Zzz\n");
            fflush(stdout);
            break;
        }
    }
    sem_destroy(&chef_sync_buffes);
    pthread_exit(NULL);
}

// Chef adiciona comida nas bacias vazias dos buffets
void chef_put_food(int buffet, int meal, buffet_t* buffet_array)
{
    printf("CHEF: Reabastecendo a refeição %d do buffet %d\n", meal, buffet);
    fflush(stdout);

    // Retorna ao valor inicial da bacia, deixando-a cheia
    buffet_array[buffet]._meal[meal] = 40;
    printf("CHEF: Refeição %d do buffet %d foi reabastecida\n", meal, buffet);
    fflush(stdout);
}

// Chef observa se as bacias estão vazias e se há estudantes nos buffets
int chef_check_food()
{
    buffet_t* buffet_array = globals_get_buffets();
    int there_is_students = FALSE;
    int number_of_buffets = globals_get_number_of_buffets();

    //Percorre os buffets
    for (int i = 0; i < number_of_buffets; i++) {
        // Ṕercorre as bacias
        for (int j = 0; j < 5; j++) {
            // Verifica se a bacia está vazia
            if (buffet_array[i]._meal[j] == 0) {
                printf("CHEF: Refeição %d do buffet %d acabou!\n", j, i);
                fflush(stdout);
                // Adiciona comida se for o caso
                chef_put_food(i, j, buffet_array);
            }

            // Checa se há estudante nos buffets
            if (buffet_array[i].queue_left[j] != 0 || buffet_array[i].queue_right[j]) {
                there_is_students = TRUE;
            }
        }
    }
    return there_is_students;
}

/* --------------------------------------------------------- */
/* ATENÇÃO: Não será necessário modificar as funções abaixo! */
/* --------------------------------------------------------- */

void chef_init(chef_t *self)
{
    pthread_create(&self->thread, NULL, chef_run, NULL);
}

void chef_finalize(chef_t *self)
{
    pthread_join(self->thread, NULL);
    free(self);
}