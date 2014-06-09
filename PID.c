/*****************************************************************
 	 	 	 	 EMBARCADOS.COM.BR

 	 	 Projeto: PID Digital para microcontroladores

 	 	 Arquivo: PID.c
 	 	 Desc	: modulo contendo as funcoes e metodos para
 	 	 	 	  criacao e gerenciamento do PID.

 	 	 Autor: FSN

 *****************************************************************/

/*****************************************************************
 	 	 Includes
 *****************************************************************/
#include <string.h>
#include <stdint.h>
#include "PID.h"
/*****************************************************************
 	 	 Prototipos internos:
 *****************************************************************/
void PID_vComputaCoef(int32_t lKp,
					  int32_t lKi,
					  int32_t lKd,
					  ObjetoPID_t *xPID);

void PID_vLimpaHistorico(ObjetoPID_t *xPID);


int32_t PID_lCalculaErro(int32_t lSetPoint,
						 int32_t lSensor);

void PID_vCalcula(ObjetoPID_t * xPID , int32_t lSensor);

void PID_vAtualizaHistorico(ObjetoPID_t *xPID);

/*****************************************************************
 	 	 Funcoes:
 *****************************************************************/

/*****************************************************************
		Funcao: PID_vIncializa()

		Desc: inicializa o objeto PID alocado e o coloca
			  pronto para uso.

		Param: xPID - ponteiro paraobjeto PID que se deseja
					  inicializar.

		Retorna: N/A
 *****************************************************************/
void PID_vInicializa(PID_t *xPID)
{
	//checa por ponteiro nulo:
	if((PID_t * )0 == xPID) return;


	//Objeto valido, vamos limpar seu conteudo:
	memset((char *)xPID, 0x00, sizeof(ObjetoPID_t));

	//inicializa ponteiro de funcao para os metodos
	xPID->PID_vCalculaCoeficientes = &PID_vComputaCoef;

	xPID->PID_vComputaNovo = &PID_vCalcula;

	xPID->PID_vLimpaHistorico = &PID_vLimpaHistorico;

	xPID->PID_vAtualizaHistorico = &PID_vAtualizaHistorico;

	//terminado, encerra funcao:
	return;
}
/*****************************************************************
		Funcao: PID_vComputaCoef()

		Desc: Calcula os coeficientes do controle PID
		      Apontado pelo usuario.

		Param: lKp - Ganho proporcional
			   lKi - Ganho integral
			   lKd - Ganho derivativo
			   xPID - Objeto PID

		Retorna: N/A
 *****************************************************************/
void PID_vComputaCoef(int32_t lKp,
					  int32_t lKi,
					  int32_t lKd,
					  ObjetoPID_t *xPID)
{
	//checa se o ponteiro do objeto nao e nulo:
	if((ObjetoPID_t *)0 == xPID) return;

	//Satura coeficientes maiores que o maximo permitido:
	if(lKp > PID_MAX) lKp = PID_MAX;
	if(lKi > PID_MAX) lKi = PID_MAX;
	if(lKd > PID_MAX) lKd = PID_MAX;


	//Satura coeficientes para o minitmo permitido:
	if(lKp < PID_MIN) lKp = PID_MIN;
	if(lKi < PID_MIN) lKi = PID_MIN;
	if(lKd < PID_MIN) lKd = PID_MIN;


	//Computa coeficientes para o objeto xPID:
	xPID->lA0 = 1;
	xPID->lA1 = -1;
	xPID->lB0 = lKp+lKi+lKd;

	//satura B0 se necesario:
	if(xPID->lB0 > PID_MAX) xPID->lB0 = PID_MAX;
	if(xPID->lB0 < PID_MIN) xPID->lB0 = PID_MIN;

	xPID->lB1 = (-lKp + lKi - ((2 * lKd)) );

	//satura B1 se necesario:
	if(xPID->lB1 > PID_MAX) xPID->lB1 = PID_MAX;
	if(xPID->lB1 < PID_MIN) xPID->lB1 = PID_MIN;

	xPID->lB2 = -lKd;

	//Bloco PID configurado, ja pode ser operado.
}

/*****************************************************************
		Funcao: PID_vLimpaHistorico()

		Desc: Limpa o historico das variaveis do PID
			  funciona como um reset

		Param: xPID - Objeto PID desejado.

		Retorna: N/A
 *****************************************************************/
void PID_vLimpaHistorico(ObjetoPID_t *xPID)
{
	//Checa se o ponteiro e nulo:
	if((ObjetoPID_t*)0 == xPID) return;


	{
		//aloca um contador temporario:
		uint32_t dI;

		//limpa historico do erro
		for(dI = 0; dI < 3 ; dI++) xPID->lErro[dI] = 0;

		//limpa historico de saida:
		for(dI = 0; dI < 2 ; dI++) xPID->lSaida[dI] = 0;

	}
}
/*****************************************************************
		Funcao: PID_sCalculaErro()

		Desc: Calcula um novo valor de erro para o PID

		Param: sSetPoint - Valor desejado
		       sSensor   - Valor Lido

		Retorna: sErro - -32768 a 32768
 *****************************************************************/
int32_t PID_lCalculaErro(int32_t lSetPoint,
						 int32_t lSensor)
{
	int32_t lErro = 0;

	//Satura valores maiores que o permitido se necessario:
	if(lSetPoint > PID_MAX) lSetPoint = PID_MAX;
	if(lSensor  > PID_MAX) lSensor = PID_MAX;

	//Satura valores menores que o permitido se necessario:
	if(lSetPoint < PID_MIN) lSetPoint = PID_MIN;
	if(lSensor < PID_MIN) lSensor = PID_MIN;

	//Computa o erro:
	lErro = lSetPoint - lSensor;

	//satura se precisar:
	if(lErro > PID_MAX) lErro = PID_MAX;
	if(lErro < PID_MIN) lErro = PID_MIN;

	//encerra sub:
	return(lErro);
}
/*****************************************************************
		Funcao: PID_vCalcula()

		Desc: Calcula um novo valor para a saida PID

		Param: xPID - Objeto PID desejado.

		Retorna: N/A
 *****************************************************************/
void PID_vCalcula(ObjetoPID_t * xPID , int32_t lSensor)
{
	//computa o erro:
	xPID->lErro[0] = PID_lCalculaErro(xPID->lSetPoint, lSensor);

	//Erro computado atualiza saida computando o PID:
	xPID->lAcumulador = xPID->lB0 * xPID->lErro[0] +
						xPID->lB1 * xPID->lErro[1] +
						xPID->lB2 * xPID->lErro[2] -
						xPID->lSaida[1];

	//Escala acumulador e coloca na SAIDA:
	xPID->lSaida[0] = (xPID->lAcumulador >> 16);

	//Satura se necessario o valor de saida para o maximo:
	if(xPID->lSaida[0] > PID_MAX)xPID->lSaida[0] = PID_MAX;
	//Satura se necessario o valor de saida para o minimo:
	if(xPID->lSaida[0] < PID_MIN)xPID->lSaida[0] = PID_MIN;
}
/*****************************************************************
		Funcao: PID_vAtualizaHistorico()

		Desc: Atualiza o esatdo das variaveis de saida e erro
			 essa funcao deve ser chamada toda vez depois
			 que uma saida for compuada.

		Param: xPID - Objeto PID desejado.

		Retorna: N/A
 *****************************************************************/
void PID_vAtualizaHistorico(ObjetoPID_t *xPID)
{

	//checa se o ponteiro eh nulo:
	if((ObjetoPID_t *)0 == xPID) return;

	//atualiza historico de erro primeiro:
	xPID->lErro[2] = xPID->lErro[1];
	xPID->lErro[1] = xPID->lErro[0];

	//atualiza historico da saida:
	xPID->lSaida[1] = xPID->lSaida[0];

}
