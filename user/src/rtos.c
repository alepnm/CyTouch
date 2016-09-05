/* ========================================
 *
 * Copyright VENTMATIKA, 2016
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF Ventmatika.
 *
 * ========================================
*/
#include "rtos.h"


/* ----------------------------- Externals ----------------------------------*/
/* ------------------------------ Globals -----------------------------------*/
/* ------------------------------- Locals -----------------------------------*/
typedef struct _task
{   
   void (*pFunc) (void);               // ��������� �� �������
   uint16_t delay;                     // �������� ����� ������ �������� ������
   uint16_t period;                    // ������ ������� ������
   uint8_t run;                        // ���� ���������� ������ � �������
}task;


volatile task TaskArray[MAXnTASKS];      // ������� �����
volatile uint8_t TaskArrTail;            // "�����" �������

/*=============================================================================
 ������������� ������������
=============================================================================*/
void Shed_Init()
{
  uint8_t i;
	
  for(i = 0u; i<MAXnTASKS; i++)
  {	
	  TaskArray[i].pFunc  = 0x0000;
	  TaskArray[i].delay  = 0u;
	  TaskArray[i].period = 0u;
	  TaskArray[i].run    = 0u;	//false
  }  
  TaskArrTail = 0u;  // "�����" � 0  
}

/*=============================================================================
 ���������� ������ � ������
=============================================================================*/
void Shed_SetTask (void (*taskFunc)(void), uint16_t taskDelay, uint16_t taskPeriod)
{

  if( !taskFunc ) return;

  uint8_t IntState;

  IntState = CyEnterCriticalSection();

  uint8_t i;

  for(i = 0u; i < TaskArrTail; i++)                     // ����� ������ � ������� ������
  {
	if(TaskArray[i].pFunc == taskFunc)              // ���� �����, �� ��������� ����������
	{
	  TaskArray[i].delay  = taskDelay;
	  TaskArray[i].period = taskPeriod;
	  TaskArray[i].run    = 0u; //false	
	
	  CyExitCriticalSection(IntState);
	  return;                                       // �������, �������
	}
  }

  if (TaskArrTail < MAXnTASKS)                      // ���� ����� ������ � ������ ��� �
  {                                                 //���� �����,�� ���������
	TaskArray[TaskArrTail].pFunc  = taskFunc;
	TaskArray[TaskArrTail].delay  = taskDelay;
	TaskArray[TaskArrTail].period = taskPeriod;
	TaskArray[TaskArrTail].run    = 0u;//false  	
	TaskArrTail++;                                  // ����������� "�����"
  }

  CyExitCriticalSection(IntState);
}


/*=============================================================================
 �������� ������ �� ������
=============================================================================*/
void Shed_DeleteTask (void (*taskFunc)(void))
{
  uint8_t IntState;

  IntState = CyEnterCriticalSection();
	
  uint8_t i;

  for(i = 0u; i<TaskArrTail; i++)             // �������� �� ������ �����
  {
	if(TaskArray[i].pFunc == taskFunc)      // ���� ������ � ������ �������
	{
	  if(i != (TaskArrTail - 1u))            // ��������� ��������� ������ �� ����� ���������
	  {
		TaskArray[i] = TaskArray[TaskArrTail - 1u];
	  }
	  TaskArrTail--;                        // ��������� ��������� "������"
	
	  CyExitCriticalSection(IntState);
	  return;
	}
  }

  CyExitCriticalSection(IntState);
}


/*=============================================================================
 ��������� ������������, ���������� �� �������� �����
=============================================================================*/
void Shed_DispatchTask()
{
   void (*function) (void);

   uint8_t i;

   for (i = 0u; i<TaskArrTail; i++)                    // �������� �� ������ �����
   {
	 if (TaskArray[i].run == (1u))                  // ���� ���� �� ���������� �������,
	 {                                              // ���������� ������, �.�. ��                                                   
		 function = TaskArray[i].pFunc;             // ����� ���������� ����� ���������� ������
		 if(TaskArray[i].period == (0u))               // ���� ������ ����� 0 
		 {                                                     
			Shed_DeleteTask(TaskArray[i].pFunc);    // ������� ������ �� ������,            
		 }
		 else
		 {
			TaskArray[i].run = 0u;               // ����� ������� ���� �������
			if(!TaskArray[i].delay)
			{                                       // ���� ������ �� �������� �������� ������ ��
			   TaskArray[i].delay = TaskArray[i].period - (1u); 
			}                                       // ������ ��� ���� ����� ������� �����  
		 }
		 (*function)();                             // ��������� ������
	  }
   }
}


/*=============================================================================
 ���� ������������. �������� � ����������� ������ ms
 ������������ �������
=============================================================================*/
CY_INLINE void Shed_Tick()
{
  uint8_t i;
	
  for (i = 0u; i<TaskArrTail; i++)    // �������� �� ������ �����
    {
	    if  (TaskArray[i].delay == 0u)    // ���� ����� �� ���������� �������
	    {    
	        TaskArray[i].run = 1u;          // ������� ���� �������,
	    }
	    else
	    {
	        TaskArray[i].delay--;             // ����� ��������� �����
	    }
    }   
}
