//**********************************************************************//
//                                                                      //
//                     Atividade 2                                      //
//     IOT11 - Sistemas Operacionais de Tempo Real                      //
//     Aluno: Alessandro Ribeiro Lage                                   //
//     Data: 17/08/2022                                                 //
//                                                                      //
//**********************************************************************//
//                                                                      //
//   Estamos vivendo tempos tenebrosos, com grandes animosidades        // 
//   entre as lideranças mundiais, polarização política, quebra de      //
//   paradigmas e tabus sociais... Em resumo, um caldeirão de           //
//   pólvora!                                                           //
//   Alta tecnologia é sempre bom, mas o simples pode ser a única       //
//   saída em momentos de crise. Por isso, desenvolva uma solução com   //
//   o FreeRTOS que faça a transformação de texto em código morse,      //
//   representado por ‘.’ e ‘- ́. Também deverá ser reproduzido de        //
//   forma visual, através da representação de LED, usado nos exemplos. //
//   A aplicação deverá ser composta por pelo menos 4 tasks, sendo:     //
//   • Leitura de teclado;                                              //
//   • Processador de texto, codificando a frase sempre que for         //
//     identificado a tecla enter;                                      //
//   • Codificador para morse em caracteres ‘.’ e ‘-’;                  //
//   • Reprodutor visual do código morse gerado.                        //
//   A troca de dados entre as tasks deverão acontecer através          //
//   de QUEUEs.                                                         //
//                                                                      //
//**********************************************************************//
//   Comentários adicionais:                                            //
//   - Criada tarefa de leitura de teclado                              //
//                                                                      //
//                                                                      //
//**********************************************************************//


#include <stdio.h>
#include <pthread.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"

/* Local includes. */
#include "console.h"
#include <termios.h>

//Definição das prioridades de cada Task
#define TASK1_PRIORITY 1 //Prioridade da tarefa 1
#define TASK2_PRIORITY 1 //Prioridade da tarefa 2
#define TASK3_PRIORITY 1 //Prioridade da tarefa 3
#define TASK4_PRIORITY 1 //Prioridade da tarefa 4


//Definições do programa
#define BLACK "\033[30m" /* Black */
#define RED "\033[31m"   /* Red */
#define GREEN "\033[32m" /* Green */
#define DISABLE_CURSOR() printf("\e[?25l")
#define ENABLE_CURSOR() printf("\e[?25h")

#define clear() printf("\033[H\033[J")
#define gotoxy(x, y) printf("\033[%d;%dH", (y), (x))


typedef struct
{
    int pos;
    char *color;
    int period_ms;
} st_led_param_t;

st_led_param_t green = {
    6,      //Define a posição onde o LED vai aparecer
    GREEN, // Define a cor do LED
    250};  // Define cada estado do LED verde fica ativo por 250ms
st_led_param_t red = {
    13,   // Define a posição onde o LED vai aparecer
    RED,  // Define a cor do LED
    100}; // Define o estado de acesso do LED vermelho deve durar 100ms

TaskHandle_t Task1_hdl, Task2_hdl, Task3_hdl, Task4_hdl;

//Task para leitura do telcado
static void prvTask_getChar(void *pvParameters)
{
    char key;
    int n;
    uint32_t notificationValue_getChar;

    /* I need to change  the keyboard behavior to
    enable nonblock getchar */
    struct termios initial_settings,
        new_settings;

    tcgetattr(0, &initial_settings);

    new_settings = initial_settings;
    new_settings.c_lflag &= ~ICANON;
    new_settings.c_lflag &= ~ECHO;
    new_settings.c_lflag &= ~ISIG;
    new_settings.c_cc[VMIN] = 0;
    new_settings.c_cc[VTIME] = 1;

    tcsetattr(0, TCSANOW, &new_settings);
    /* End of keyboard configuration */


    for (;;)
    {
        int stop = 0;
        key = getchar();
        if (key=='1'||key=='2'||key=='3'||key=='4'||key=='5'||key=='6'||key=='7'||key=='8'||key=='9'||key=='0')
        {
            key='n'; //verifica se foi digitado um número

        }
        xTaskNotifyWait(
            0x00, //não limpa nenhum bit de notificação na entrada
            0x00, //não limpa nenhum bit de notificação na saída
            &notificationValue_getChar,
            1);//espera um Tick
            
        if(notificationValue_getChar == 99)
        {   switch (key)
                {
                    case '+': //se digitado a tecla '+', volta ao funcionamento normal dos Leds e teclado
                        vTaskResume(greenTask_hdlr); //retoma a tarefa do Led verde
                        vTaskResume(redTask_hdlr);   //retoma a tarefa do Led vermelho
                        vTaskResume(getCharTask_hdlr);  //retoma a tarefa de leitura do teclado
                        xTaskNotify(greenTask_hdlr, 0UL, eSetValueWithOverwrite);    //notifica tarefa do Led vermelho ao estado original
                        xTaskNotify(redTask_hdlr, 0UL, eSetValueWithOverwrite);      //notifica tarefa do Led verde ao estado original
                        xTaskNotify(getCharTask_hdlr, 0UL, eSetValueWithOverwrite);  //notifica tarefa de leitura teclado ao estado original
                        break;
                }
        }
        else
        {
            switch (key)
            {
                case 'n': // se digitado número retoma a tarefa do Led Vermelho (ou seja, faz o Led vermelho piscar por 100ms e apagar)
                    vTaskResume(redTask_hdlr);
                    break;
                case '*': //se digitado '*', Led verde vai permanecer apagado, Led vermelho acesso e leitura de teclado notificada
                {
                    xTaskNotify(getCharTask_hdlr, 99UL, eSetValueWithOverwrite); //Notifica tarefa de teclado sobre a condição atual
                    xTaskNotify(greenTask_hdlr, 1UL, eSetValueWithOverwrite);    //Envia notificação para tarefa Led verde para parar de piscar apagado
                    vTaskResume(redTask_hdlr);                                   //Retoma a tarefa Led vermelho
                    xTaskNotify(redTask_hdlr, 1UL, eSetValueWithOverwrite);      //Notifica a tarefa Led vermelho para o Led permanecer acesso    
                    break;
                }
                case 'k': //se digitado k interrompe o programa para efeito de testes
                    stop = 1;
                    break;
            }
        }
        if (stop)
        {
            break;
        }
        vTaskDelay(100/ portTICK_PERIOD_MS); //intervalo de tempo para aguargar após leitura do teclado = 100ms
    }
    tcsetattr(0, TCSANOW, &initial_settings);
    ENABLE_CURSOR();
    exit(0);
    vTaskDelete(NULL);
}

void app_run(void)
{

   // clear();
   // DISABLE_CURSOR();
   // printf(
   //     "╔═════════════════╗\n"
  //      "║                 ║\n"
   //     "╚═════════════════╝\n");


    xTaskCreate(Task_leitura_teclado, "Ler_Teclado", configMINIMAL_STACK_SIZE, NULL, TASK1_PRIORITY, &Task1_hdl); //Task1 = Leitura dos dados do teclado
    //xTaskCreate(Task_processador_texto, "Processa_Texto", configMINIMAL_STACK_SIZE, NULL, TASK2_PRIORITY, &Task2_hdl); //Task2 = Processador de texto
    //xTaskCreate(Task_codificador_morse, "Codificador_Morse", configMINIMAL_STACK_SIZE, NULL, TASK2_PRIORITY, &Task2_hdl); //Task2 = Codificado texto em morse
    //xTaskCreate(Task_morse_led, "Morse_Led", configMINIMAL_STACK_SIZE, NULL, TASK2_PRIORITY, &Task2_hdl); //Task2 = Saida morse em Led


    /* Start the tasks and timer running. */
    vTaskStartScheduler();

    /* If all is well, the scheduler will now be running, and the following
     * line will never be reached.  If the following line does execute, then
     * there was insufficient FreeRTOS heap memory available for the idle and/or
     * timer tasks      to be created.  See the memory management section on the
     * FreeRTOS web site for more details. */
    for (;;)
    {
    }
}