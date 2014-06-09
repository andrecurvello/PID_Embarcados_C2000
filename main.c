/*****************************************************************
 	 	 	 	 EMBARCADOS.COM.BR

 	 	 Projeto: PID Digital para microcontroladores

 	 	 Arquivo: main.c
 	 	 Desc	: modulo principal do software

 	 	 Autor: FSN

 *****************************************************************/
#include <stdint.h>
#include "F2802x_Device.h"
#include "PID.h"

/*****************************************************************
 	 	 Defines
 *****************************************************************/


/****************************************************************
 	 	 Prototipos de funcoes locais
 ****************************************************************/
void vInicializaSistema(void);
void vInicializaClock(void);
void vInicializaEpwm(void);
void vInicializaCpuTimer(void);
void vInicializaAdc(void);
void vIniciaConversaoAD(void);
__interrupt void vTimerIsr(void);

/*****************************************************************
 	 	 Variaveis desse modulo
 *****************************************************************/
PID_t 		xMeuPID;
uint32_t    dSensor;

/*****************************************************************
 	 	 Funcoes
 *****************************************************************/
/******************************************************************
		Funcao: vIniciaConversaoAD()
		Desc: Dispara o inicio da conversao analogica para
			  digital.

		param: N/A
		retorna: N/A
 *****************************************************************/
void vIniciaConversaoAD(void)
{
	//dispara o trigger.
	AdcRegs.ADCSOCFRC1.all |= 0x01;
}

/******************************************************************
		Funcao: vInicializaClock()
		Desc: Inicializa clock do C2000

		param: N/A
		retorna: N/A
 *****************************************************************/
void vInicializaClock(void)
{
	//habilita a edicao de regs protegidos:
	EALLOW;

	//Coloca o socilador interno de 10MHz para rodar:
	SysCtrlRegs.CLKCTL.all = 0x00000000;

	//Bypass a PLL
	SysCtrlRegs.PLLSTS.all |=  (1 << 6);

	SysCtrlRegs.PLLSTS.all &=  ~(0x03 << 7);


	//Configura novo divisor pro clock:
	SysCtrlRegs.PLLCR.all = 0x0C;

	//Aguarda a PLL trancar:
	while(!(SysCtrlRegs.PLLSTS.all & 0x01));

	//Trancou, entao reescreve divsel:
	SysCtrlRegs.PLLSTS.all |= (0x02 << 7);

	//A partir desse ponto o clock ja eh 60MHz.
	EDIS;

}
/******************************************************************
		Funcao: vInicializaEpwm()
		Desc: Inicializa Epwm do C2000, nessa aplicacao
			  usamos PWM1A:B e PWM2A

		param: N/A
		retorna: N/A
 *****************************************************************/
void vInicializaEpwm(void)
{
	EALLOW;

	//Habilita clock do modulo Epwm:
	SysCtrlRegs.PCLKCR1.all |= (1<<0) | (1<<1);

	//Derruba clock do sincronismo:
	SysCtrlRegs.PCLKCR0.all &= ~(1<<2) ;

	//Habilitaclock do GPIOA:
	SysCtrlRegs.PCLKCR3.all |= (1 << 13) | (1 << 9);

	//GPIO0:2 funcionam como EPWM:
	GpioCtrlRegs.GPAMUX1.all |= 0x15;


	//COnfigura qualificador:

	GpioCtrlRegs.GPACTRL.all &= ~(0xFF);
	GpioCtrlRegs.GPAQSEL1.all |= 0x3F;

	//Configura direcao do IO:
	GpioCtrlRegs.GPADIR.all |= 0x07;

	//Direcao do IO analogico:
	GpioCtrlRegs.AIODIR.all |= 0x04;

	//Configura EPWM1 @ 32KHz:

	//Seta o peiodo:
	EPwm1Regs.TBPRD = (60000000) / 150000;
	//Seta fase:
	EPwm1Regs.TBPHS.all = 0;
	//Seta config:
	EPwm1Regs.TBCTL.all = (0x03 << 14) | (1 << 13) | (0x03 << 4);

	EPwm1Regs.TBCTL.all &= ~(0x03);
	//Seta os dutycicles:
	EPwm1Regs.CMPA.half.CMPA = 0;
	EPwm1Regs.CMPB = 0;

	//seta o controle de carga dos registros:
	EPwm1Regs.CMPCTL.all = 0x0000;
	EPwm1Regs.AQCTLA.all = (1 << 4) | (0x02 << 2);
	EPwm1Regs.AQCTLB.all = (1 << 8) | (0x02 << 2);

	//Seta o peiodo @ 32KHz:
	EPwm2Regs.TBPRD = 60000000 / 150000;
	//Seta fase:
	EPwm2Regs.TBPHS.all = 0;
	//Seta config:
	EPwm2Regs.TBCTL.all = (0x03 << 14) | (1 << 13) | (0x03 << 4);

	EPwm2Regs.TBCTL.all &= ~(0x03);
	//Seta os dutycicles:
	EPwm2Regs.CMPA.half.CMPA = 0;
	EPwm2Regs.CMPB = 0;

	//seta o controle de carga dos registros:
	EPwm2Regs.CMPCTL.all = 0x0000;
	EPwm2Regs.AQCTLA.all = (1 << 4) | (0x02 << 2);
	EPwm2Regs.AQCTLB.all = (1 << 8) | (0x02 << 2);

	//Sincroniza o clock do PWM:
	SysCtrlRegs.PCLKCR0.all |= (1<<2);

	EDIS;
}
/******************************************************************
		Funcao: vInicializaCpuTImer()
		Desc: Inicializa timer2 do C2000, para interromper cada
			  64KHz.

		param: N/A
		retorna: N/A
 *****************************************************************/
void vInicializaCpuTimer(void)
{
	EALLOW;

	//Habilitaclock do timer2:
	SysCtrlRegs.PCLKCR3.all |= (1 << 13) | (1 << 9);

	//Mantem timer2 parado, mas ja o configura paar auto reload:
	CpuTimer2Regs.TCR.all |= (1 << 4) | (0x03 << 10) | (1 << 5) | (0x01 << 14);

	//Configura Periodo do timer2:
	CpuTimer2Regs.PRD.all = (60000000/58000);

	//Derruba PIE
	PieCtrlRegs.PIECTRL.all &= ~(0x01);

	//Faz a vector table apontar para a rotina de ISR:
	PieVectTable.TINT2 = &vTimerIsr;

	//Habilita PIE:
	PieCtrlRegs.PIECTRL.all |= 0x01;

	//Habilia interrupcao global do timer:
	IER |= (1 << 13);

	//Habilita timer2:
	CpuTimer2Regs.TCR.all &= ~(1 << 4);


	EDIS;
}
/******************************************************************
		Funcao: vInicializaAdc()
		Desc: Inicializa 3 canais do AD  do C2000

		param: N/A
		retorna: N/A
 *****************************************************************/
void vInicializaAdc(void)
{
	//Variaveis auxiliares:
	uint16_t wI = 0;

	EALLOW;

	//Habilita clock do conversor A/D:
	SysCtrlRegs.PCLKCR0.all |= (1 << 3);

	//Derruba o AD:
	AdcRegs.ADCCTL1.all &= ~(1 << 14);

	//Configura A/D:
	AdcRegs.ADCCTL1.all = (1<<5) | (1<<6) | (1 << 7);

	//Habilita:
	AdcRegs.ADCCTL1.all |= (1 << 14);

	//Aguarda ~ 1ms:
	for(wI = 0; wI < 20000; wI++);

	//Configura a SOC desejada a conversao:

	//Soc0 maior prioridade:
	AdcRegs.SOCPRICTL.all |= 0x01;

	//configura modo de amostragem:
	AdcRegs.ADCSAMPLEMODE.all &= ~(0x01);

	//Configura o sequenciador de conversao:
	AdcRegs.ADCSOC0CTL.all = 0x06 | (0x0C << 6);

	EDIS;

}
/******************************************************************
		Funcao: vInicializaSistema()
		Desc: Inicializa todo o hardware desejado do C2000

		param: N/A
		retorna: N/A
 *****************************************************************/
void vInicializaSistema(void)
{
	//Derruba interrupcoes:
	DINT;

	//Confugura clock:
	vInicializaClock();

	//Inicializa EPWM:
	vInicializaEpwm();

	//Configura ADC:
	vInicializaAdc();

	//Configura timer2:
	vInicializaCpuTimer();

	//Dispara a primeira conversao:
	vIniciaConversaoAD();


	//Debug:
	ERTM;

	//Habilia interrupcoes:
	EINT;


}
/******************************************************************
		Funcao: main()
		Desc: Funcao principal do sistema

		param: N/A
		retorna: N/A
 *****************************************************************/
int main(void)
{

	//Auxiares:
	uint16_t wI;

	//Inicializa PID:
	PID_vInicializa(&xMeuPID);

	//Computa parametros:
	//KP = 50
	//KI = 15
	//KD = 100
	xMeuPID.PID_vCalculaCoeficientes(50, 15, 100, &xMeuPID.xParamPID);

	//Limpa historico:
	xMeuPID.PID_vLimpaHistorico(&xMeuPID.xParamPID);

	//Inicializa HW:
	vInicializaSistema();
	


	for(;;)
	{
		//Coloca setpoint inicial, exemplo, VBUS = 19.2V
		xMeuPID.xParamPID.lSetPoint = (992 << 4);
	};
}
/******************************************************************
		Funcao: vTimerIsr()
		Desc: Interrupcao do timer2

		param: N/A
		retorna: N/A
 *****************************************************************/
__interrupt void vTimerIsr(void)
{

	//Efetua leitura do sensor, escala para 16bits:
	dSensor = (AdcResult.ADCRESULT0 << 4);

	//Computa novo PID:
	xMeuPID.PID_vComputaNovo(&xMeuPID.xParamPID, (int32_t)dSensor );

	//COloca valor no Epwm e escala:
	//EPwm2Regs.CMPA.half.CMPA = 10;//(int16_t)(512 + (xMeuPID.xParamPID.lSaida[0] >> 7));

	//Atualiza historico:
	xMeuPID.PID_vAtualizaHistorico(&xMeuPID.xParamPID);

	//Dispara nova conversao:
	vIniciaConversaoAD();
}
