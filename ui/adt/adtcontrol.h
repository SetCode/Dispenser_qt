#ifndef ADTCONTROL_H
#define ADTCONTROL_H

#include <Windows.h>
#include <QtWidgets>
#include <QDebug>
#include <Eigen/Dense>
#include <iostream>
#include <stdio.h>
#include <adt8949.h>

using namespace std;
using namespace Eigen;

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
	float            speed;					  // �ٶ�
	float            extra_offset_z;	      // ����ƫ��Z
	bool             open;					  // �Ƿ񿪽�
	int              openAdvance;			  // ��ǰ����ʱ��
	int              openDelay;               // �ӳٿ���ʱ��
	bool             close;                   // �Ƿ�ؽ�
	int              closeAdvance;            // ��ǰ�ؽ�ʱ��
	int              closeDelay;              // �Ӻ�ؽ�ʱ��
	QString			 type;					  // ����

	_CCDGlue()
	{
		name = "";                    // ��λ����
		description = "";             // ��λ����
		X = 0.000;					  // X
		Y = 0.000;					  // Y
		Z = 0.000;                    // Z
		center_X = 0.000;			  // Բ��X
		center_Y = 0.000;			  // Բ��Y
		speed = 0.000;				  // �Ƿ�������
		extra_offset_z = 0.000;		  // ����ƫ��Z		
		open = false;				  // �Ƿ񿪽�
		openAdvance = 0;			  // ��ǰ����ʱ��
		openDelay = 0;                // �ӳٿ���ʱ��
		close = false;                // �Ƿ�ؽ�
		closeAdvance = 0;             // ��ǰ�ؽ�ʱ��
		closeDelay = 0;               // �Ӻ�ؽ�ʱ��
		type = "null";				  // ����
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
		speed = other.speed;
		extra_offset_z = other.extra_offset_z;
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
	Z = 3,
	A = 4
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

// ������ƿ�����
void load_card();


// ���û�ԭģʽ
void set_home_mode();

// ���û�ԭ�ٶ�
void set_home_speed();

// ���ԭ
void home_axis(int axis);

// �ȴ���ԭ���
void wait_axis_homeOk(int axis);



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

// ���������˶�, ������(0+, 1-), �����ٶ�����, ����ǰ�����ٶ�, ģʽ
void move_axis_continue(int axis, int dir);


// ��������˶�, ���ٶ�, ���Ӽ���, ��������λ
void move_axis_abs(int axis, float pos, float speed, float acc, float dec);

// ��������˶�, ���ٶ�, ���Ӽ���, ��������λ
void move_axis_offset(int axis, float distance, float speed, float acc, float dec);

// ���������˶�, ������(0+, 1-), ���ٶ�, ���Ӽ���, ��������λ
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
void move_inp_abs_helix2(float pos_x, float pos_y, float pos_z, float center_x, float center_y);

// X, YԲ���岹 by pos_x, pos_y, center_x, center_y
void move_inp_abs_arc2(float pos_x, float pos_y, float center_x, float center_y);

// �����Ƿ������˶� ���� 0ֹͣ, 1�˶�
bool axis_isMoving(int axis);

// �Ƿ������˶� ���� 0ֹͣ, 1�˶�
bool card_isMoving();

// �ȴ�����ֹͣ
void wait_axis_stop(int axis);

// �ȴ�������ֹͣ
void wait_allaxis_stop();

// �ȴ��岹���
void wait_inp_finish();

// ���ò������ٶ�
void set_stepAxis_speed(float speed);

// �ȴ�������ֹͣ
void wait_stepAxis_stop();


void asyncSleep(unsigned int msec);



#endif // ADTCONTROL_H