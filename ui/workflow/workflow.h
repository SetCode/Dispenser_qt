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

private:
	QThreadPool thread_pool;
	QMutex mutex_serial;
	
private:	// ��ͣ
	bool is_estop;
	bool close_thread_watch_estop;
	QFuture<void> future_thread_watch_estop;
	void thread_watch_estop();

private:	// ��λ
	bool is_reset_ok;
	bool close_thread_watch_reset;
	QFuture<void> future_thread_watch_reset;
	void thread_watch_reset();

private:	// ���ӵ㽺
	bool is_glue_ing;
	bool close_thread_watch_glue;
	QFuture<void> future_thread_watch_glue;
	void thread_watch_glue();

private:	// ���Ӳ���
	bool is_clearNeedle_ing;
	bool close_thread_watch_clearNeedle;
	QFuture<void> future_thread_watch_clearNeedle;
	void thread_watch_clearNeedle();

private:	// �����Ž�
	bool is_dischargeGlue_ing;
	bool close_thread_watch_dischargeGlue;
	QFuture<void> future_thread_watch_dischargeGlue;
	void thread_watch_dischargeGlue();
	
private:	// ��ʼ
	bool is_start_ok;
	bool close_thread_watch_start;
	QFuture<void> future_thread_watch_start;	
	void thread_watch_start();


private:	// �㽺1,2,3
	bool is_config_gluel;		
	bool is_config_glue2;
	bool is_config_glue3;

private:	// �㽺̩��
	bool is_config_glue_teda = true;
	bool is_glue_teda_ok;
	bool start_thread_glue_teda;
	bool close_thread_glue_teda;
	QFuture<void> future_thread_glue_teda;
	void thread_glue_teda();

private:	// �㽺̩�����
	QFuture<void> future_thread_glue_teda_test;
	void thread_glue_teda_test();

private:	// BTN CCD�궨
	bool is_thread_btn_ccd_calib_ing;
	bool close_thread_btn_ccd_calib;
	QFuture<void> future_thread_btn_ccd_calib;
	void thread_btn_ccd_calib();

private:	// BTN CCD����
	bool is_thread_btn_ccd_runEmpty_ing;
	bool close_thread_btn_ccd_runEmpty;
	QFuture<void> future_thread_btn_ccd_runEmpty;
	void thread_btn_ccd_runEmpty();

private:	// BTN ���ܲ��㽺
	bool is_thread_btn_runEmpty_ing;
	bool close_thread_btn_runEmpty;
	QFuture<void> future_thread_btn_runEmpty;
	void thread_btn_runEmpty();

private:	// BTN ��ͷ����
	bool is_thread_btn_clearNeedle_ing;
	bool close_thread_btn_clearNeedle;
	QFuture<void> future_thread_btn_clearNeedle;
	void thread_btn_clearNeedle();

private:	// BTN �Զ��Ž�
	bool is_thread_btn_dischargeGlue_ing;
	bool close_thread_btn_dischargeGlue;
	QFuture<void> future_thread_btn_dischargeGlue;
	void thread_btn_dischargeGlue();

private:	// BTN ��ͷУ׼1
	bool is_btn_needleCalib1_ing;
	bool close_thread_needleCalib1;
	QFuture<void> future_thread_needleCalib_1;
	void thread_needleCalib_1();

private:	// BTN ��ͷУ׼2
	bool is_btn_needleCalib2_ing;
	bool close_thread_needleCalib2;
	QFuture<void> future_thread_needleCalib_2;
	void thread_needleCalib_2();

private:	// ͨѶ
	QTcpSocket  *socket_ccd;
	QString receivedMsg_ccd;
	void socket_ccd_receive();

	QSerialPort *serial_laser;
	QString receivedMsg_laser;
	bool close_thread_serialLaserReceive;
	void thread_serialLaserReceive();


private:	// ��λ
	QSqlTableModel *model_general;	// ��λģ��
	QSqlTableModel *model_glue1;
	QSqlTableModel *model_glue2;
	QSqlTableModel *model_glue3;

	// ���е�λ ��ͨ���źŲ���ˢ�µ�λ, ����Ҫ������������
	QMap<QString, PointRun>  allpoint_pointRun;
	QMap<QString, PointRun>  getAllRunPointInfo();
	
	// ��վ��λ ��ͨ���źŲ���ˢ�µ�λ, ����Ҫ������������
	QVector<CCDGlue> vec_ccdGlue_1;
	QVector<CCDGlue> vec_ccdGlue_2;
	QVector<CCDGlue> vec_ccdGlue_3;
	QVector<CCDGlue> getCCDGluePoint2Vector(int index);

	// ����ƽ�ƾ���
	MatrixXf CalCCDGluePoint(const QVector<CCDGlue> vector_ccdGlue, const float offset_x, const float offset_y);
	// ����ƽ����ת����
	MatrixXf CalCCDGluePoint(const QVector<CCDGlue> vector_ccdGlue, const float offset_x, const float offset_y, const float offset_angle, const float org_x, const float org_y);
	// ����ƽ����ת��λ
	void CalCCDGlueCenterPoint(float center_pos[2], const float center_x, const float center_y, const float offset_x, const float offset_y, const float offset_angle, const float org_x, const float org_y);

private:	// ƫ��
	float distance_ccd_needle_x;
	float distance_ccd_neddle_y;

	float distance_ccd_laser_x;
	float diatance_ccd_laser_y;

	float distance_laser_needle_x;
	float distance_laser_needle_y;
	float distance_laser_needle_z;

	float distance_needle_z;

	// ��ͨ���źŲ���ˢ��ƫ����, ����Ҫ������������
	float calib_offset_x;
	float calib_offset_y;
	float calib_offset_z;

	// ��ͨ���źŲ���ˢ�����ĵ�, ����Ҫ������������
	float org_ccdglue_x[3];
	float org_ccdglue_y[3];


private:
	// �ƶ�����, by point
	bool move_point_name(QString pointname, int z_flag = 0);


private:
	void writRunningLog(QString str);	
	QString getCurrentTime();

	bool getLaserData(float laser, int limit_ms);
	bool getVisionData(float offset_x, float offset_y, float offset_z, int limit_ms);

public slots:	// �Զ���� ���� Operation
	void on_clicked_check_flowConfig();
	void on_clicked_btn_saveDistanceOffset();

	void on_clicked_btn_ccd_calib();
	void on_clicked_btn_ccd_runEmpty();
	void on_clicked_btn_runEmpty();
	void on_clicked_btn_clearNeedle();
	void on_clicked_btn_dischargeGlue();
	void on_clicked_btn_needleCalib_1();
	void on_clicked_btn_needleCalib_2();

public slots:	// �Զ���� ���� PointDebug
	void on_changedSqlModel(int index);

signals:		// �Զ����ź� ���� Operation
	void changedRundataLabel(QString str);	
	void changedRundataText(QString str);	
	void changedDistanceOffset();
	void changedOffsetChart(float x, float y, float A);	

	void changedDischargeGlue();
};

#endif // WORKFLOW_H
