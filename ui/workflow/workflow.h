#ifndef WORKFLOW_H
#define WORKFLOW_H

#include <QObject>
// ������
#include <QtWidgets>
#include <Windows.h>
#include <QDebug>

// �߳̿�
#include <QtConcurrent>

// ͨѶ��
#include <QtSerialPort/qserialport.h>
#include <QtSerialPort/qserialportinfo.h>
#include <QtSerialPort/qtserialportversion.h>
#include <QtNetwork>

// ��ͼ��
#include <QtCharts>

// ��ѧ��
#include <QtMath>
#include <math.h>

// ���ݿ�
#include <QtSql>

// �˶�����
#include "../adt/adtcontrol.h"

// ��λģ��
enum MODELPOINT
{
	GENERAL = 0,
	GLUE1   = 1,
	GLUE2   = 2,
	GLUE3   = 3
};

// Z��������������
enum ZMOVETYPE
{
	NORMAL = 0,
	BEFORE = 1,
	AFTER = -1
};

// �㽺��λCCD��Ұ����
enum CCDORG
{
	GLUE1ORG = 0,
	GLUE2ORG = 1,
	GLUE3ORG = 2
};

class Workflow : public QObject
{
    Q_OBJECT
public:
    explicit Workflow(QObject *parent = nullptr);
	~Workflow();

private:
	void setConfig();
	void setPoint();
	void setIOStatus();
	void setThread();
	void setConnect();
	void setCommunication();

public:
	QThreadPool thread_pool;
	
public:		// ��λ
	bool is_reset_ok;
	bool start_thread_watch_reset;
	bool close_thread_watch_reset;
	QFuture<void> future_thread_watch_reset;
	void thread_watch_reset();
	
public:		// ��ʼ
	bool is_start_ok;
	bool start_thread_watch_start;
	bool close_thread_watch_start;
	QFuture<void> future_thread_watch_start;	
	void thread_watch_start();

public:		// ֹͣ
	bool is_stop_ok;
	bool start_thread_watch_stop;
	bool close_thread_watch_stop;
	QFuture<void> future_thread_watch_stop;
	void thread_watch_stop();

public:		// ��ͣ
	bool is_estop_ok;
	bool start_thread_watch_estop;
	bool close_thread_watch_estop;
	QFuture<void> future_thread_watch_estop;	
	void thread_watch_estop();

public:		// ͨѶ
	QTcpSocket  *socket_ccd;
	QString receivedMsg_ccd;
	void socket_ccd_receive();


	QSerialPort *serial_laser;
	QString receivedMsg_laser;

	bool close_thread_serialLaserReceive;
	void thread_serialLaserReceive();

public:		// ��������
	bool is_workflow_ok;
	bool start_thread_workflow;
	bool close_thread_workflow;
	QFuture<void> future_thread_workflow;		
	void thread_workflow();

public:		// ������
	bool is_exchangeTrays_ok;
	bool start_thread_exchangeTrays;
	bool close_thread_exchangeTrays;
	QFuture<void> future_thread_exchangeTrays;
	void thread_exchangeTrays();

public:		// �㽺1
	bool is_config_gluel;		// �Ƿ�����  glue1
	bool is_gluel_ok;			
	bool start_thread_glue_1;
	bool close_thread_glue_1;
	QFuture<void> future_thread_glue_1;
	void thread_glue_1();

public:		// �㽺2
	bool is_config_glue2;		// �Ƿ�����  glue1
	bool is_glue2_ok;
	bool start_thread_glue_2;
	bool close_thread_glue_2;
	QFuture<void> future_thread_glue_2;
	void thread_glue_2();

public:		// �㽺3
	bool is_config_glue3;		// �Ƿ�����  glue1
	bool is_glue3_ok;
	bool start_thread_glue_3;
	bool close_thread_glue_3;
	QFuture<void> future_thread_glue_3;
	void thread_glue_3();

public:		// �㽺̩��
	bool is_config_glue_teda = true;
	bool is_glue_teda_ok;
	bool start_thread_glue_teda;
	bool close_thread_glue_teda;
	QFuture<void> future_thread_glue_teda;
	void thread_glue_teda();

public:		// �㽺̩��
	QFuture<void> future_thread_glue_teda_test;
	void thread_glue_teda_test();

public:		// ���ܵ㽺�켣
	bool is_ccdGlue3_ok;
	bool start_thread_ccd_glue_1;
	bool close_thread_ccd_glue_1;
	QFuture<void> future_ccd_glue_1;
	void thread_ccd_glue_1();

public:
	// ��ͨ���źŲ���ˢ�µ�λ, ����Ҫ������������
	QVector<CCDGlue> vec_ccdGlue_1;
	QVector<CCDGlue> vec_ccdGlue_2;
	QVector<CCDGlue> vec_ccdGlue_3;

	// ��ͨ���źŲ���ˢ�����ĵ�, ����Ҫ������������
	float org_ccdglue_x[3];
	float org_ccdglue_y[3];

	// ��ͨ���źŲ���ˢ��ƫ����, ����Ҫ������������
	float calib_offset_x;
	float calib_offset_y;
	float calib_offset_z;

public:
	float distance_ccd_needle_x;
	float distance_ccd_neddle_y;

	float distance_ccd_laser_x;
	float diatance_ccd_laser_y;

	float distance_laser_needle_x;
	float distance_laser_needle_y;
	float distance_laser_needle_z;

public:		// У��
	bool is_calibNeedle_ok;
	bool start_thread_calibNeedle;
	bool close_thread_calibNeedle;
	QFuture<void> future_thread_calibNeedle;
	void thread_calibNeedle();

public:		// �彺
	bool is_clearNeedle_ok;
	bool start_thread_clearNeedle;
	bool close_thread_clearNeedle;
	QFuture<void> future_thread_clearNeedle;
	void thread_clearNeedle();


public:		// ��λ����
	QSqlTableModel *model_general;
	QSqlTableModel *model_glue1;
	QSqlTableModel *model_glue2;
	QSqlTableModel *model_glue3;

signals:	// �Զ����ź�
	// To Operation
	void changedRundataLabel(QString str);	
	void changedRundataText(QString str);	
	void changedOffsetChart(float x, float y, float A);	
	void changedOffset(float offset_x, float offset_y, float offset_z);
	
public slots:	// �����ⲿ�ź�
	// ���� Operation
	void on_changedConfigGlue(bool glue1, bool glue2, bool glue3);			
	void on_changedConfigGlueOffset(float offset_x, float offset_y, float offset_z);
	
	// ���� PointDebug
	void on_changedSqlModel(int index);		


public:
	// дLog�ļ�
	void writRunningLog(QString str);
	QString getCurrentTime();


public:		// ��ȡ��λ
	QMap<QString, PointGeneral> allpoint_general;
	QMap<QString, PointGlue> allpoint_glue1;
	QMap<QString, PointGlue> allpoint_glue2;
	QMap<QString, PointGlue> allpoint_glue3;
	QMap<QString, PointRun>  allpoint_pointRun;
	
	QMap<QString, PointGeneral> getAllGeneralPointInfo();
	QMap<QString, PointGlue> getAllGluePointInfo(int index);
	QMap<QString, PointRun>  getAllRunPointInfo();
	
	QVector<CCDGlue> getCCDGluePoint2Vector(int index);

	MatrixXf CalCCDGluePoint(const QVector<CCDGlue> vector_ccdGlue, const float offset_x, const float offset_y);
	MatrixXf CalCCDGluePoint(const QVector<CCDGlue> vector_ccdGlue, const float offset_x, const float offset_y, const float offset_angle, const float org_x, const float org_y);

	void CalCCDGlueCenterPoint(float center_pos[2], const float center_x, const float center_y, const float offset_x, const float offset_y, const float offset_angle, const float org_x, const float org_y);

	float wSpeed;
	float wAcc;
	float wDec;

	void set_speed(float speed, float acc, float admode);
	void set_speed(float speed, float acc);

	bool move_point_name(QString pointname, int type, int z_flag);
	bool move_point_name(QString pointname, int z_flag = 0);		// �ƶ�����, by point

};

#endif // WORKFLOW_H