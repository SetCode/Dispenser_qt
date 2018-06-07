#ifndef ADTCONTROL_H
#define ADTCONTROL_H

#include <Windows.h>
#include <QtWidgets>
#include <QDebug>
#include <iostream>
#include <stdio.h>
#include <Eigen/Dense>

using namespace std;
using namespace Eigen;

#include "adt8949.h"


// CCD�㽺��(���)
typedef struct _CCDGlue
{
	QString          name;                    // ��λ����
	QString          description;             // ��λ����
	float            X;					      // X
	float            Y;						  // Y
	float            Z;                       // Z
	float            center_X;				  // Բ��X
	float            center_Y;				  // Բ��Y
	bool			 laser;					  // �Ƿ�������
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
		center_X = 0.000;			  // Բ��X
		center_Y = 0.000;			  // Բ��Y
		laser = false;				  // �Ƿ�������
		open = false;				  // �Ƿ񿪽�
		openAdvance = 0;			  // ��ǰ����ʱ��
		openDelay = 0;                // �ӳٿ���ʱ��
		close = false;                // �Ƿ�ؽ�
		closeAdvance = 0;             // ��ǰ�ؽ�ʱ��
		closeDelay = 0;               // �Ӻ�ؽ�ʱ��
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
		center_X = other.center_X;
		center_Y = other.center_Y;
		laser = other.laser;
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

enum ADMODE
{
	S = 0,
	T = 1,
	INDEX = 2,
	TRIAN = 3
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


// ��ȡ�������λ��
float get_current_pos_axis(int axis);


// �����������ٶ�, ����ʼ�ٶ�, ���Ӽ���, ��ģʽ
void set_speed_mode(float startv, float speed, float acc, unsigned short mode);

// ���õ����ٶ�, �����, ����ʼ�ٶ�, ���Ӽ���, ��ģʽ
void set_axis_speed_mode(int axis, float startv, float speed, float acc, unsigned short mode);

// ��������˶�, �����ٶ�����, ����ǰ�����ٶ�, ģʽ
void move_axis_abs(int axis, float pos);

// ��������˶�, �����ٶ�����, ����ǰ�����ٶ�, ģʽ
void move_axis_offset(int axis, float distance);

// ���������˶�, �����ٶ�����, ����ǰ�����ٶ�, ģʽ
void move_axis_continue(int axis, int dir);


// �����ƶ�, ��������λ
void move_axis_abs(int axis, float pos, float speed, float acc, float dec);

// ����ƶ�, ���ٶ�, ���Ӽ���, ��������λ
void move_axis_offset(int axis, float distance, float speed, float acc, float dec);

// �����˶�, ������(0+, 1-), ���ٶ�, ���Ӽ���, ��������λ
void move_axis_continue(int axis, int dir, float speed, float acc, float dec);


// ���ò岹�˶�, ����ʼ�ٶ�, ���Ӽ���, ��ģʽ
void set_inp_speed_mode(float startv, float speed, float acc, unsigned short mode);

// �������ٲ岹�˶��ٶ�, ���ٶ�
void set_inp_speed(float speed);

// X, Y, Z����ֱ�߲岹, by x_pos, y_pos, z_pos
void move_inp_abs_line3(float x_pos, float y_pos, float z_pos);

// X, Y����ֱ�߲岹
void move_inp_abs_line2(float x_pos, float y_pos);

// X, YԲ���岹, Z��ֱ�߲岹 by pos_x, pos_y, pos_z, center_x, center_y
void move_imp_abs_helix2(float pos_x, float pos_y, float pos_z, float center_x, float center_y);

// X, YԲ���岹 by pos_x, pos_y, center_x, center_y
void move_inp_abs_arc2(float pos_x, float pos_y, float center_x, float center_y);

// �˶�, 
bool axis_isMoving(int axis);

// �ȴ�����ֹͣ
void wait_axis_stop(int axis);

// �ȴ�������ֹͣ
void wait_allaxis_stop();

// �ȴ��岹���
void wait_inp_finish();

#endif // ADTCONTROL_H