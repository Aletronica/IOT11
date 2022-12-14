//*********************************************************************//
//                                                                     //
//           Atividade Extra - Evoluindo a Demo 4                      //
//     IOT11 - Sistemas Operacionais de Tempo Real                     //
//     Aluno: Alessandro Ribeiro Lage                                  //
//     Data: 18/08/2022                                                //
//                                                                     //
//*********************************************************************//
//                                                                     //
//    Descrição:                                                       //
//   - Alterar a Demo_4 de tal forma que ela sempre pare com o LED     //        
//     apagado                                                         //
//   - Tecla q = apaga o Led Vermelho                                  //
//   - Tecla w = religa o Led Vermelho (Piscando a 500ms)              //
//   - Tecla a = apaga o Led Verde                                     //
//   - Tecla s = religa o Led Verde (Piscando a 1s)                    //
//   - Tecla k = interrompe o programa (kill)                          //
//                                                                     //
//*********************************************************************//


#include <stdio.h>
#include <pthread.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"

/* Local includes. */
#include "console.h"

#define TASK1_PRIORITY 0
#define TASK2_PRIORITY 0
#define TASK3_PRIORITY 1

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
    6,
    GREEN,
    500};
st_led_param_t red = {
    13,
    RED,
    1000};

TaskHandle_t greenTask_hdlr, redTask_hdlr;

#include <termios.h>

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

        switch (key)
        {
        case 'a':
            vTaskResume(greenTask_hdlr);
            break;
        case 's':
            xTaskNotify(greenTask_hdlr, 99UL, eSetValueWithOverwrite); //Notifica a tarefa do Led verde para entrar em suspensão
            //vTaskSuspend(greenTask_hdlr);
            break;
        case 'q':
            vTaskResume(redTask_hdlr);
            break;
        case 'w':
            xTaskNotify(redTask_hdlr, 1UL, eSetValueWithOverwrite);  //Notifica a tarefa do Led vermelho para entrar em suspensão
            //vTaskSuspend(redTask_hdlr);
            break;
        case 'k':
            stop = 1;
            break;
        }
        if (stop)
        {
            break;
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    tcsetattr(0, TCSANOW, &initial_settings);
    ENABLE_CURSOR();
    exit(0);
    vTaskDelete(NULL);
}

static void prvTask_greenled(void *pvParameters)
{
    // pvParameters contains LED params
    st_led_param_t *led = (st_led_param_t *)pvParameters;
    portTickType xLastWakeTime = xTaskGetTickCount();
    uint32_t notificationValue_green;
    for (;;)
    {
        // console_print("@");
        gotoxy(led->pos, 2);
        printf("%s⬤", led->color);
        fflush(stdout);
        vTaskDelay(led->period_ms / portTICK_PERIOD_MS);
        // vTaskDelayUntil(&xLastWakeTime, led->period_ms / portTICK_PERIOD_MS);

        gotoxy(led->pos, 2);
        printf("%s ", BLACK);
        fflush(stdout);
        vTaskDelay(led->period_ms / portTICK_PERIOD_MS);
        if (xTaskNotifyWait(                           //Aguarda notificação para o Led Verde
            0x00,
            0x00,
            &notificationValue_green,                   //Recebe notificação para mudar de estado
            1)) //espera um Tick
        {
                if(notificationValue_green == 99)       //Confirma se foi notificado
                {
                   vTaskSuspend(greenTask_hdlr);        //Muda para estado suspenso se confirmada a notificação. Suspende com Led verde apagado.
                }
        }
    }

    vTaskDelete(NULL);
}

static void prvTask_redled(void *pvParameters)
{
    // pvParameters contains LED params
    st_led_param_t *led = (st_led_param_t *)pvParameters;
    portTickType xLastWakeTime = xTaskGetTickCount();
    uint32_t notificationValue_red;
    for (;;)
    {
        // console_print("@");
        gotoxy(led->pos, 2);
        printf("%s⬤", led->color);
        fflush(stdout);
        vTaskDelay(led->period_ms / portTICK_PERIOD_MS);
        // vTaskDelayUntil(&xLastWakeTime, led->period_ms / portTICK_PERIOD_MS);

        gotoxy(led->pos, 2);
        printf("%s ", BLACK);
        fflush(stdout);
        vTaskDelay(led->period_ms / portTICK_PERIOD_MS);
        if (xTaskNotifyWait(                           //Aguarda notificação para o Led vermelho
            0x00,
            0x00,
            &notificationValue_red,                   //Recebe notificação para mudar de estado
            1)) //espera um Tick
        {
                if(notificationValue_red == 01)       //Confirma se foi notificado
                {
                   vTaskSuspend(redTask_hdlr);        //Muda para estado suspenso se confirmada a notificação. Suspende com Led vermelho apagado.
                }
        }
    }

    vTaskDelete(NULL);
}

void app_run(void)
{

    clear();
    DISABLE_CURSOR();
    printf(
        "╔═════════════════╗\n"
        "║                 ║\n"
        "╚═════════════════╝\n");

    xTaskCreate(prvTask_greenled, "LED_green", configMINIMAL_STACK_SIZE, &green, TASK1_PRIORITY, &greenTask_hdlr);
    xTaskCreate(prvTask_redled, "LED_red", configMINIMAL_STACK_SIZE, &red, TASK2_PRIORITY, &redTask_hdlr);
    xTaskCreate(prvTask_getChar, "Get_key", configMINIMAL_STACK_SIZE, NULL, TASK3_PRIORITY, NULL);

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