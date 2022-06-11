#include <stdlib.h>

#include "chef.h"
#include "config.h"
#include "globals.h"

/* Chef checa o buffet e adiciona comida enquanto tem estudantes se servindo,
quando não há mais estudantes no buffet ou na fila, o chef encerra */
void *chef_run()
{
    queue_t* fila_fora = globals_get_queue();

    while (TRUE)
    {
        // Chef checa se há comida no buffet e estudantes se servindo
        int there_is_students = chef_check_food();
        // Caso a fila e o buffet estiverem sem estudantes, o chef encerra
        if (!there_is_students && fila_fora->_length == 0) {
            printf("chef foi pra casa a mimir");
            break;
        }
    }
    
    pthread_exit(NULL);
}

// Chef adiciona comida nas bacias vazias dos buffets
void chef_put_food(int buffet, int meal, buffet_t* buffet_array)
{
    printf("enchendo bacia %d do buffet %d", meal, buffet);
    msleep(5000);
    // Retorna ao valor inicial da bacia, deixando-a cheia
    buffet_array[buffet]._meal[meal] == 40;
    printf("bacia %d do buffet %d cheia", meal, buffet);
}

// Chef observa se as bacias estão vazias e se há estudantes nos buffets
int chef_check_food()
{
    buffet_t* buffet_array = globals_get_buffets();
    int there_is_students = FALSE;

    //Percorre os buffets
    for (int i = 0; i < config.buffets; i++) {
        // Ṕercorre as bacias
        for (int j = 0; j < 5; j++) {
            // Verifica se a bacia está vazia
            if (buffet_array[i]._meal[j] == 0) {
                printf("bacia %d do buffet %d vazia\n", j, i);
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