#ifndef ADTCONTROL_H
#define ADTCONTROL_H

#include <Windows.h>
#include <QtWidgets>
#include <QDebug>
#include <iostream>
#include <stdio.h>
using namespace std;


#include "adt8949.h"

// CCD�㽺��(���)
typedef struct _CCDGlue
{
	QString          name;                    // ��λ����
	QString          description;             // ��λ����
	float            X;					      // X
	float            Y;						  // Y
	float            Z;                       // Z
	bool             open;					  // �Ƿ񿪽�
	int              openAdvance;			  // ��ǰ����ʱ��
	int              openDelay;               // �ӳٿ���ʱ��
	bool             close;                   // �Ƿ�ؽ�
	int              closeAdvance;            // ��ǰ�ؽ�ʱ��
	int              closeDelay;              // �Ӻ�ؽ�ʱ��
	int			     type;					  // ����

	_CCDGlue()
	{
		name = "";                    // ��λ����
		description = "";             // ��λ����
		X = 0.000;					  // X
		Y = 0.000;					  // Y
		Z = 0.000;                    // Z
		open = false;				  // �Ƿ񿪽�
		openAdvance = 0;			  // ��ǰ����ʱ��
		openDelay = 0;                 // �ӳٿ���ʱ��
		close = false;                // �Ƿ�ؽ�
		closeAdvance = 0;             // ��ǰ�ؽ�ʱ��
		closeDelay = 0;                // �Ӻ�ؽ�ʱ
		type = 0;					  // ����
	}

	_CCDGlue & operator = (const _CCDGlue &other)
	{
		if (this == &other)
		{
			return *this;
		}
		name = other.name;
		description = other.description;
		X = other.X;
		Y = other.Y;
		Z = other.Z;
		open = other.open;
		openAdvance = other.openAdvance;
		openDelay = other.openDelay;
		close = other.close;
		closeAdvance = other.closeAdvance;
		closeDelay = other.closeDelay;
		type = other.type;
		return *this;
	}
}CCDGlue;

// ʵ�ʵ㽺��
typedef struct _PointGlue
{
	QString          name;                    // ��λ����
	QString          description;             // ��λ����
	float            X;					      // X
	float            Y;						  // Y
	float            Z;                       // Z
	bool             open;					  // �Ƿ񿪽�
	int              openAdvance;			  // ��ǰ����ʱ��
	int              openDelay;               // �ӳٿ���ʱ��
	bool             close;                   // �Ƿ�ؽ�
	int              closeAdvance;            // ��ǰ�ؽ�ʱ��
	int              closeDelay;              // �Ӻ�ؽ�ʱ��
	int			     type;					  // ����

	_PointGlue()
	{
		name = "";                    // ��λ����
		description = "";             // ��λ����
		X = 0.000;					  // X
		Y = 0.000;					  // Y
		Z = 0.000;                    // Z
		open = false;				  // �Ƿ񿪽�
		openAdvance = 0;			  // ��ǰ����ʱ��
		openDelay = 0;                 // �ӳٿ���ʱ��
		close = false;                // �Ƿ�ؽ�
		closeAdvance = 0;             // ��ǰ�ؽ�ʱ��
		closeDelay = 0;                // �Ӻ�ؽ�ʱ
		type = 0;					  // ����
	}

	_PointGlue & operator = (const _PointGlue &other)
	{
		if (this == &other)
		{
			return *this;
		}
		name = other.name;
		description = other.description;
		X = other.X;
		Y = other.Y;
		Z = other.Z;
		open = other.open;
		openAdvance = other.openAdvance;
		openDelay = other.openDelay;
		close = other.close;
		closeAdvance = other.closeAdvance;
		closeDelay = other.closeDelay;
		type = other.type;
		return *this;
	}
}PointGlue;

// ��ͨ��λ
typedef struct _PointGeneral
{
	QString          name;                    // ��λ����
	QString          description;             // ��λ����
	float            X;					      // X
	float            Y;						  // Y
	float            Z;                       // Z

	_PointGeneral()
	{
		name = "";                    // ��λ����
		description = "";             // ��λ����
		X = 0.000;					  // X
		Y = 0.000;					  // Y
		Z = 0.000;                    // Z
	}

	_PointGeneral & operator = (const _PointGeneral &other)
	{
		if (this == &other)
		{
			return *this;
		}
		name = other.name;
		description = other.description;
		X = other.X;
		Y = other.Y;
		Z = other.Z;
		return *this;
	}
}PointGeneral;

// ���е�λ ���� name desc x y z
typedef struct _PointRun
{
	QString          name;                    // ��λ����
	QString          description;             // ��λ����
	float            X;					      // X
	float            Y;						  // Y
	float            Z;                       // Z

	_PointRun()
	{
		name = "";                    // ��λ����
		description = "";             // ��λ����
		X = 0.000;					  // X
		Y = 0.000;					  // Y
		Z = 0.000;                    // Z
	}

	_PointRun & operator = (const _PointRun &other)
	{
		if (this == &other)
		{
			return *this;
		}
		name = other.name;
		description = other.description;
		X = other.X;
		Y = other.Y;
		Z = other.Z;
		return *this;
	}
}PointRun;

enum AXISNUM
{
	X = 1,
	Y = 2,
	Z = 3
};

// ��ʼ�����ƿ� ��һ�ε��òų�ʼ��, �ڶ��ε���ʱ���ص�һ�γ�ʼ���Ľ��
int init_card();

// ����ֹͣ
void stop_axis_dec(int axis);


// ֹͣ��, by axis
void stop_axis(int axis);

// ֹͣ������
void stop_allaxis();

// ��ͣ
void estop();




// ��ȡ�����״̬, by bit
int read_in_bit(int bit);

// ��ȡ�����״̬, by bit
int read_out_bit(int bit);

// д�������״̬, by bit
void write_out_bit(int bit, int status);

// �ı������״̬, by bit
void change_out_bit(int bit);




// ���û�ԭģʽ
void set_home_mode();

// ���û�ԭ�ٶ�
void set_home_speed();

// ���ԭ
void home_axis(int axis);

// �ȴ���ԭ���
void wait_axis_homeOk(int axis);



// �ٶ�, �����, ���Ӽ���
void set_axis_speed(int axis, float speed, float acc, float dec);

// �ƶ�, ���ٶ�, ���Ӽ���, ��������λ
void move_axis_abs(int axis, float pos, float speed, float acc, float dec);

// �ƶ�, ���ٶ�, ���Ӽ���, ��������λ
void move_axis_offset(int axis, float distance, float speed, float acc, float dec);

// �����˶�, ������(0+, 1-), ���ٶ�, ���Ӽ���, ��������λ
void move_axis_continue(int axis, int dir, float speed, float acc, float dec);


float get_current_pos_axis(int axis);


// �˶�, 
bool axis_isMoving(int axis);

// �ȴ��˶�����
void wait_axis_stop(int axis);



#endif // ADTCONTROL_H