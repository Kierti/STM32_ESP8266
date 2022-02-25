#ifndef __ESP8266_H
#define __ESP8266_H

#include "tim.h"
#include "usart.h"
#include "stdbool.h"
#define RxBufferSize 2048 //���ջ����С

/************ESP8266�������Ͷ���**************/
/*ESP8266��ǰ����ģʽ */
typedef enum{
	NOINIT,			//δ��ʼ��
	STA,			//�ӻ�ģʽ
	AP,				//����ģʽ
	APSTA,			//�ӻ�+����ģʽ
	DISCONNECT		//�Ͽ�����
}mode_set;
/*����ִ�е�״̬ */
typedef enum{
	Idle,			//����
	Busy,			//��æ
	Sent			//������ϣ��ȴ�����
}uart_state_set;
/* ����ִ�еĽ�� */
typedef enum{
		Fail,        //ʧ��
		Success,     //�ɹ�
		Timeout      //��ʱ
}result;
/*
 ** 2.0�汾��1.0����֮�ϣ���ԭ�ȵ�ESP8266ָ���ת��Ϊ����ָ�룬�Ҳ���ESP8266�ṹ����
 */
typedef struct{
	UART_HandleTypeDef *huart;                  //ESP8266���õĴ���
	DMA_HandleTypeDef *dma_rx;     				//���ڴ��ڽ��յ�DMA
	TIM_HandleTypeDef *tim;        				//���ڼ�¼�Ƿ�ʱ�Ķ�ʱ��
	mode_set mode;                 				//ESP8266�Ĺ���ģʽ
	uart_state_set uart_state;    				//��¼����ͨ�ŵĴ��ڵ�״̬
	char RxBuffer[RxBufferSize]; 				//���ջ���
	char AckBuffer[RxBufferSize]; 				//�����Ӧ���ݵĻ���
	result (*send)(char *str, const char* response, uint16_t timeout);
	void (*sendNoAck)(uint8_t *str);
	result (*sendCmd)(const char *str, const char* response, uint16_t timeout);
	void (*quitTrans)(void);
	void (*RxCallback)(void);
	void (*setTimeout)(uint16_t ms);
	bool (*isTimeout)(void);
	void (*delay)(uint16_t ms);
	bool (*isResponded)(const char* response);
	result (*init)(void);
}ESP8266;

/*ESP8266�ṹ�������ʼ��*/
void wifiInit(void);
/* �������� */
result send(char *str, const char* response, uint16_t timeout);
/* ��������(��Ҫ����Ӧ) */
void sendNoAck(uint8_t *str);
/* ����ָ�� */
result sendCmd(const char *str, const char* response, uint16_t timeout);
/* �˳�͸��ģʽ */
void quitTrans(void);
/* ���ջص����������Լ���ӣ� */
void RxCallback(void);
/* ���ó�ʱʱ�� */
void setTimeout(uint16_t ms);
/* �ж��Ƿ�ʱ */
bool isTimeout(void);
/* �ӳٺ��� */
void delay(uint16_t ms);
/* �ж��Ƿ���Ӧ */
bool isResponded(const char* response);
/* ��ʼ�� */
result init(void);
/* �മ���ض������ */
void print(UART_HandleTypeDef* huart, const char* buf, ...);

extern ESP8266 wifi;
#endif
