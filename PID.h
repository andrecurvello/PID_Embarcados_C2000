/*****************************************************************
 	 	 	 	 EMBARCADOS.COM.BR

 	 	 Projeto: PID Digital para microcontroladores

 	 	 Arquivo: PID.h
 	 	 Desc	: modulo interface para acesso as funcoes
 	 	 	 	  PID.

 	 	 Autor: FSN

 *****************************************************************/
#ifndef PID_H_
#define PID_H_

//Sem tipo padrao, usa o do C.
#include <stdint.h>

/*****************************************************************
 	 	 Defines
 *****************************************************************/
//Taxa de amostragem do PID em HZ.
#define PID_TX_AMOSTRAGEM  160000

//Valor maximo de saida do controlador PID
#define PID_MAX  32768

//Valor minimo de saida do controlador PID
#define PID_MIN -32768

//Shift escalador de saida:
#define PID_SHIFT_SAIDA 16
/*****************************************************************
 	 	 Estruturas e afins
 *****************************************************************/
//Define um tipo para metodo:
typedef void MetodoPID;

//objeto para controlador PID:
typedef struct
{
	//Variaveis:

	//Coeficientes do controlador PID:
	int32_t  lB0;

	int32_t  lB1;

	int32_t  lB2;

	int32_t  lA0;

	int32_t  lA1;

	//Acumulador de saida:
	int32_t  lAcumulador;

	//Estado de saida:
	int32_t  lSaida[2];

	//Estado de erro:
	int32_t  lErro[3];

	//Set point:
	int32_t  lSetPoint;

}ObjetoPID_t;

typedef struct
{
	ObjetoPID_t xParamPID;

	//Metodos:

	//Calcula coeficientes
	MetodoPID (*PID_vCalculaCoeficientes)(int32_t sKp, int32_t sKi, int32_t sKd, ObjetoPID_t *xThis);

	//Limpa historico:
	MetodoPID (*PID_vLimpaHistorico)(ObjetoPID_t *xThis);

	//Computa novo valor de saida:
	MetodoPID (*PID_vComputaNovo)(ObjetoPID_t * xThis , int32_t sSensor);

	//Atualiza historico:
	MetodoPID (*PID_vAtualizaHistorico)(ObjetoPID_t *xThis);
}PID_t;

/*****************************************************************
 	 	 Prototipos das funcoes
 *****************************************************************/
extern void PID_vInicializa(PID_t *xPID);

/*****************************************************************
 	 	 Fim do arquivo
 *****************************************************************/
#endif /* PID_H_ */
