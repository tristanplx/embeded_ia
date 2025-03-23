/**
  ******************************************************************************
  * @file    app_x-cube-ai.c
  * @brief   Corps du programme d’IA pour le modèle network
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * Tous droits réservés.
  *
  * Ce logiciel est distribué selon les termes précisés dans le fichier LICENSE
  * à la racine du composant logiciel. S'il n'y a pas de fichier LICENSE, il est
  * fourni "AS-IS".
  *
  ******************************************************************************
  */

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>

#include "app_x-cube-ai.h"
#include "main.h"
#include "ai_datatypes_defines.h"
#include "network.h"
#include "network_data.h"

/* USER CODE BEGIN includes */
#include "usart.h"  // Pour avoir la déclaration de UART_HandleTypeDef

extern UART_HandleTypeDef huart2; // Déclaration externe de huart2

#define SYNCHRONISATION    0xAB
#define ACKNOWLEDGE        0xCD
#define TIMEOUT            1000
#define CLASS_NUMBER       5  // Nombre de classes mis à jour pour correspondre au Python
#define INPUT_BUFFER_SIZE  AI_NETWORK_IN_1_SIZE_BYTES
/* USER CODE END includes */

/* IO buffers ----------------------------------------------------------------*/
#if !defined(AI_NETWORK_INPUTS_IN_ACTIVATIONS)
AI_ALIGNED(4) ai_i8 data_in_1[AI_NETWORK_IN_1_SIZE_BYTES];
ai_i8* data_ins[AI_NETWORK_IN_NUM] = {
  data_in_1
};
#else
ai_i8* data_ins[AI_NETWORK_IN_NUM] = {
  NULL
};
#endif

#if !defined(AI_NETWORK_OUTPUTS_IN_ACTIVATIONS)
AI_ALIGNED(4) ai_i8 data_out_1[AI_NETWORK_OUT_1_SIZE_BYTES];
ai_i8* data_outs[AI_NETWORK_OUT_NUM] = {
  data_out_1
};
#else
ai_i8* data_outs[AI_NETWORK_OUT_NUM] = {
  NULL
};
#endif

/* Activations buffers -------------------------------------------------------*/
AI_ALIGNED(32)
static uint8_t pool0[AI_NETWORK_DATA_ACTIVATION_1_SIZE];
ai_handle data_activations0[] = { pool0 };

/* AI objects ----------------------------------------------------------------*/
static ai_handle network = AI_HANDLE_NULL;
static ai_buffer* ai_input;
static ai_buffer* ai_output;

static void ai_log_err(const ai_error err, const char *fct)
{
  /* USER CODE BEGIN log */
  if (fct)
    printf("TEMPLATE - Erreur (%s) - type=0x%02x code=0x%02x\r\n", fct,
           err.type, err.code);
  else
    printf("TEMPLATE - Erreur - type=0x%02x code=0x%02x\r\n", err.type, err.code);

  do {} while (1);
  /* USER CODE END log */
}

static int ai_boostrap(ai_handle *act_addr)
{
  ai_error err;

  /* Création et initialisation de l'instance du modèle */
  err = ai_network_create_and_init(&network, act_addr, NULL);
  if (err.type != AI_ERROR_NONE) {
    ai_log_err(err, "ai_network_create_and_init");
    return -1;
  }

  ai_input = ai_network_inputs_get(network, NULL);
  ai_output = ai_network_outputs_get(network, NULL);

#if defined(AI_NETWORK_INPUTS_IN_ACTIVATIONS)
  for (int idx = 0; idx < AI_NETWORK_IN_NUM; idx++) {
    data_ins[idx] = ai_input[idx].data;
  }
#else
  for (int idx = 0; idx < AI_NETWORK_IN_NUM; idx++) {
    ai_input[idx].data = data_ins[idx];
  }
#endif

#if defined(AI_NETWORK_OUTPUTS_IN_ACTIVATIONS)
  for (int idx = 0; idx < AI_NETWORK_OUT_NUM; idx++) {
    data_outs[idx] = ai_output[idx].data;
  }
#else
  for (int idx = 0; idx < AI_NETWORK_OUT_NUM; idx++) {
    ai_output[idx].data = data_outs[idx];
  }
#endif

  return 0;
}

static int ai_run(void)
{
  ai_i32 batch;

  batch = ai_network_run(network, ai_input, ai_output);
  if (batch != 1) {
    ai_log_err(ai_network_get_error(network), "ai_network_run");
    return -1;
  }
  return 0;
}

/* USER CODE BEGIN 2 */

/**
  * @brief  Attend la synchronisation UART.
  *         Attend de recevoir l’octet SYNCHRONISATION (0xAB) du PC et répond par ACKNOWLEDGE (0xCD).
  */
void wait_for_synchronization_UART(void)
{
  uint8_t rx;
  while (1)
  {
    if (HAL_UART_Receive(&huart2, &rx, 1, TIMEOUT) == HAL_OK)
    {
      if (rx == SYNCHRONISATION)
      {
        uint8_t ack = ACKNOWLEDGE;
        HAL_UART_Transmit(&huart2, &ack, 1, TIMEOUT);
        // Lecture d'un éventuel octet supplémentaire si nécessaire (optionnel)
        break;
      }
    }
  }
}

/**
  * @brief  Acquiert les données d'entrée via UART et les copie dans le buffer d'entrée.
  * @param  data : pointeur vers le buffer d'entrée (ai_input[0].data)
  * @retval 0 si succès, 1 sinon.
  */
int acquire_and_process_data(ai_i8* data)
{
  uint8_t tmp[INPUT_BUFFER_SIZE] = {0};
  HAL_StatusTypeDef status = HAL_UART_Receive(&huart2, tmp, INPUT_BUFFER_SIZE, TIMEOUT);
  if (status != HAL_OK)
  {
    printf("Erreur de réception UART: %d\n", status);
    return 1;
  }
  // On suppose ici que les données reçues sont déjà au format attendu.
  memcpy(data, tmp, INPUT_BUFFER_SIZE);
  return 0;
}

/**
  * @brief  Post-traite la sortie du modèle et la transmet via UART.
  *         La sortie est supposée contenir CLASS_NUMBER valeurs float sur 4 octets.
  * @param  data : pointeur vers le buffer de sortie (ai_output[0].data)
  * @retval 0 si succès, 1 sinon.
  */
int post_process(ai_i8* data)
{
  uint8_t out_uint8[CLASS_NUMBER] = {0};
  for (size_t i = 0; i < CLASS_NUMBER; i++)
  {
    uint8_t temp[4] = {0};
    for (size_t j = 0; j < 4; j++)
    {
      temp[j] = ((uint8_t*)data)[i * 4 + j];
    }
    float val = *(float*)temp;
    out_uint8[i] = (uint8_t)(val * 255);
  }
  HAL_StatusTypeDef status = HAL_UART_Transmit(&huart2, out_uint8, CLASS_NUMBER, TIMEOUT);
  if (status != HAL_OK)
  {
    printf("Erreur d'envoi UART: %d\n", status);
    return 1;
  }
  return 0;
}
/* USER CODE END 2 */

/* Entry points --------------------------------------------------------------*/

void MX_X_CUBE_AI_Init(void)
{
  /* USER CODE BEGIN 5 */
  printf("\r\nTEMPLATE - Initialisation\r\n");
  ai_boostrap(data_activations0);
  /* USER CODE END 5 */
}

void MX_X_CUBE_AI_Process(void)
{
  /* USER CODE BEGIN 6 */
  int res = -1;
  uint8_t *in_data = (uint8_t*)ai_input[0].data;
  uint8_t *out_data = (uint8_t*)ai_output[0].data;

  // Synchronisation UART : attend la requête du PC
  wait_for_synchronization_UART();

  if (network)
  {
    do
    {
      // 1. Acquisition et prétraitement des données d'entrée
      res = acquire_and_process_data((ai_i8*)in_data);

      // 2. Exécution de l'inférence
      if (res == 0)
        res = ai_run();

      // 3. Post-traitement et transmission des résultats
      if (res == 0)
        res = post_process((ai_i8*)out_data);

    } while (res == 0);
  }

  if (res)
  {
    ai_error err = { AI_ERROR_INVALID_STATE, AI_ERROR_CODE_NETWORK };
    ai_log_err(err, "Process a échoué");
  }
  /* USER CODE END 6 */
}

#ifdef __cplusplus
}
#endif
