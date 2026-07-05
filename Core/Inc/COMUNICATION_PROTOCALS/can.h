#ifndef CAN_H
#define CAN_H

#define ID_CHECK 0x04901001

/*
 * esse ID está aqui meramente para simular um erro
 * não é um erro real da FCCM
 * troocar para 0x04901000, assim que o código novo
 * chegar da FT
 */
#define ID_ERROR 0x04901000 //o valor aqui é uma simulação de erro

/*
o protocolo vai ser dividido em duas partes:

um header vai ser criado para o envio de informações da fccm,
e o outro vai ser criado para apenas as mensagens de erro
*/

void config(void);
void declare_can_CHECK(void);
void declare_can_ERROR(void);
void Slice_DATA(void);
void send_FCCM(void);

#endif
