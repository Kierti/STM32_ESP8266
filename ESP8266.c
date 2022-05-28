#include "ESP8266.h"
#include "usart.h"
#include "tim.h"
#include "string.h"
#include "stdbool.h"
#include "stdlib.h"
#include "stdio.h"
#include "stdarg.h"
#include "main.h"

extern DMA_HandleTypeDef hdma_usart6_rx;

/**
 *@brief ESP8266�ṹ�������ʼ��
 *@retval None
 **/
char _buf[128];
ESP8266 wifi;
void wifiInit(void)
{
	wifi.huart=&huart6;
	wifi.dma_rx=&hdma_usart6_rx;
	wifi.tim=&htim6;
	wifi.send=send;
	wifi.sendNoAck=sendNoAck;
	wifi.sendCmd=sendCmd;
	wifi.quitTrans=quitTrans;
	wifi.RxCallback=RxCallback;
	wifi.setTimeout=setTimeout;
	wifi.isTimeout=isTimeout;
	wifi.delay=delay;
	wifi.isResponded=isResponded;
	wifi.init=init;
}

/**
 * @brief  �മ���ض���
 * @retval None
 **/
void print(UART_HandleTypeDef* huart, const char* buf, ...)
{
  char str[RxBufferSize] = {0};
  va_list v;
  va_start(v, buf);
  vsprintf(str, buf, v); 	//ʹ�ÿɱ�������ַ�����ӡ������sprintf
  HAL_UART_Transmit(huart,(uint8_t*)str,strlen(str),0xffffffff);
  va_end(v);
}

/**
 * @brief  ESP8266 ���ڽ��ջص�����
 * @retval None
 **/
void RxCallback(){
	UART_HandleTypeDef*  huart = wifi.huart; //ESP8266�õ��Ĵ���

	/* �ж��Ƿ��������ж� */
	if (__HAL_UART_GET_FLAG(huart, UART_FLAG_IDLE) != RESET) {
		__HAL_UART_CLEAR_IDLEFLAG(huart); //���IDLE�жϱ�־λ
		HAL_UART_DMAStop(huart);          // ֹͣDMA����

		//���²����ǽ����յ������ݴ��� AckBuffer ��
		int recv_end    = RxBufferSize - __HAL_DMA_GET_COUNTER(wifi.dma_rx); // �õ����յ����ַ�����
		int recv_start  = recv_end % RxBufferSize;                      // ���յ������ݵ���ʼ����
		while(wifi.RxBuffer[recv_start] == 0) { //�ҳ����յ���ʼ
			recv_start = (recv_start + 1) % RxBufferSize;
		}
		int index = recv_start;
		int i;
		for(i=0; index != recv_end; i++)
		{
			wifi.AckBuffer[i] = wifi.RxBuffer[index];
			wifi.RxBuffer[index] = 0;
			index = (index+1)%RxBufferSize;
		}
		wifi.AckBuffer[i] = 0; //�����ַ����ָ���

		//�����ã������յ�����Ӧͨ���������
		print(&huart3,"\r\n����:\r\n");
		print(&huart3,"%s",wifi.AckBuffer);
		//ʹ�ö���������
		if(wifi.uart_state == Sent) {	//������ڷ�����ϣ��ȴ�����
			wifi.uart_state = Idle; 	//��Ϊ����ģʽ
		}
		if(wifi.uart_state == Idle) { //��һ���Ǳ����ؽ��յ���Ϣ��
		//for(;;); //TODO:

		}

		/* ׼����һ��DMA���� */
		__HAL_DMA_SET_COUNTER(wifi.dma_rx, 0);
		HAL_UART_Receive_DMA(huart, (uint8_t*)wifi.RxBuffer, RxBufferSize);
	}
}

/**
 * @brief  ESP8266 ���÷���ָ������ݺ����ĵȴ���ʱʱ��
 * @note ms���ܳ���43,690
 * @retval None
 **/
void setTimeout(uint16_t ms){
	__HAL_TIM_SET_AUTORELOAD(wifi.tim, ms*3/2);  //Ҫ�����ӳ�10ms���жϣ�����Ҫ���� ARR Ϊ 10*3/2;
	__HAL_TIM_SET_COUNTER(wifi.tim, 1);          //��ΪҪ����CNT�Ƿ�Ϊ0���ж��Ƿ�ʱ��������������Ϊ1
	__HAL_TIM_ENABLE(wifi.tim);                  //������ʱ������ʱ����������Զ���0����ֹͣ����
}

/**
 * @brief  ESP8266 �ж��Ƿ�ʱ
 * @note   ������setTimeout֮�����ʹ��
 * @retval None
 **/
bool isTimeout(void){
	if(__HAL_TIM_GET_COUNTER(wifi.tim) == 0){
		return true;
	}
	else {
		return false;
	}
}


/*
 *
 * */
void sendNoAck(uint8_t *str) {
	print(&huart6,"%s",(char*)str);
//TODO: ������
	if(!(str[0] == '\r' && str[1] == '\n')) {
		print(&huart3,"\r\n����:\r\n");
		print(&huart3,"%s",(char*)str);
	}
}


result send(char *str,const char* response,uint16_t timeout){
	/* �������� */
	sendNoAck((uint8_t*)str);

	/* �ȴ���Ӧ */
	wifi.uart_state = Sent;
	setTimeout(timeout); //���ó�ʱʱ��
	while(!isTimeout() && !isResponded(response));
	wifi.uart_state = Idle;

	/* �ж�ִ�еĽ�� */
	if(isTimeout()) {
		return Timeout; //����ʧ�ܻ��߳�ʱ
	}
	else {
		return Success; //���ݳɹ�����
	}
}

result sendCmd(const char *str,const char* response,uint16_t timeout){
	/* �������� */
	sendNoAck((uint8_t*)str);
	sendNoAck((uint8_t*)"\r\n");

	/* �ȴ���Ӧ */
	wifi.uart_state = Sent;
	setTimeout(timeout); //���ó�ʱʱ��
	while(!isTimeout() && !isResponded(response));
	wifi.uart_state = Idle;

	/* �ж�ִ�еĽ�� */
	if(isResponded(response)) {
		return Success;
	}
	else {
		return Timeout; //����ʧ�ܻ��߳�ʱ
	}
}


/**
 *@brief ���Ͳ������ȵ�����
 *@param str:��������
 *@param linkID:��������ID��
 *@param length���������ݳ���
 *@retval None
 */
void sendMessage(char *str,char* linkID,char *length){
	sprintf(_buf,"AT+CIPSEND=%s,%s",linkID,length);
	sendCmd(_buf,">",200);
	send(str,"SEND OK",200);
}

/**
 * @brief  �˳�͸��ģʽ
 * @retval None
 **/
void quitTrans(void) {
	/* ���� "+++" �˳�͸��״̬ */
	sendNoAck((uint8_t*)"+"); delay(15);//���ڴ�����֡ʱ��(10ms)
	sendNoAck((uint8_t*)"+"); delay(15);
	sendNoAck((uint8_t*)"+"); delay(500);
	/* �ر�͸��ģʽ */
	sendCmd("AT+CIPMODE=0", "OK", 200);
}

/**
 * @brief  �ӳٺ���
 * @note   ͨ����ʱ��ʵ�ֵ��ӳ٣�ֻ��ʹ����quitTrans����
 * @retval None
 **/
void delay(uint16_t ms) {
	setTimeout(ms);
	while(!isTimeout());
}

/**
 * @brief  �ж��Ƿ��ESP8266�õ���������Ӧ��ĸ�����Ӵ���
 * @retval �Ƿ�õ���������Ӧ
 **/
bool isResponded(const char* response){
	int responseLen = strlen(response);
	int rxBufferLen = strlen(wifi.RxBuffer);
	for(int i=0,j=0; i<rxBufferLen; i++) {
		if(wifi.RxBuffer[i] == response[j])	{
			if(++j == responseLen) { //�ҵ��ִ� �����ҵ���Ӧ��
				return true;
			}
		}
		else{
			j = 0;
		}
	}
	return false; //δ�ҵ��Ӵ�
}

/**
 * @brief  ESP8266 ��ʼ������
 * @retval ��ʼ���Ƿ�ɹ�
 **/
result init(){
	wifiInit();
	UART_HandleTypeDef*  huart = wifi.huart; //ESP8266�õ��Ĵ���
	/* ����DMA���� */
	HAL_UART_Receive_DMA(huart, (uint8_t*)wifi.RxBuffer, RxBufferSize);
	__HAL_UART_ENABLE_IT(huart, UART_IT_IDLE); //���������ж�

	/* ע�����ʱ��ʱ����û�п�ʼ���� */
	HAL_TIM_Base_Start(wifi.tim);
	HAL_TIM_OnePulse_Init(wifi.tim, TIM_OPMODE_SINGLE);                 //����Ϊ OnePulse ģʽ
	__HAL_TIM_SET_PRESCALER(wifi.tim, HAL_RCC_GetHCLKFreq()/3/1000-1);  //���÷�Ƶ��Ϊ 56,000

	delay(100);
	return Success;
}
