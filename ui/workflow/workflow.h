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

// ���ݿ�
#include <QtSql>

// �˶�����
#include "../adt/adtcontrol.h"
#include "../io/io.h"

enum MODELPOINT
{
	GENERAL = 0,
	GLUE1   = 1,
	GLUE2   = 2,
	GLUE3   = 3
};

enum POINTTYPE
{

};

enum ZMOVETYPE
{
	NORMAL = 0,
	BEFORE = 1,
	AFTER = -1
};

class Workflow : public QObject
{
    Q_OBJECT
public:
    explicit Workflow(QObject *parent = nullptr);

private:
	void setConfig();
	void setPoint();
	void setIOStatus();
	void setThread();
	void setConnect();

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


public:		// �㽺
	bool is_config_gluel;
	bool is_config_glue2;
	bool is_config_glue3;
	void thread_glue_1();
	void thread_glue_2();
	void thread_glue_3();


public:		// У��
	float glue_offset_x;
	float glue_offset_y;

	bool is_calibNeedle_ok;
	bool start_thread_calibNeedle;
	bool close_thread_calibNeedle;
	QFuture<void> future_thread_calibNeedle;
	void thread_calibNeedle();

public:		// ��λ����
	QSqlTableModel *model_general;
	QSqlTableModel *model_glue1;
	QSqlTableModel *model_glue2;
	QSqlTableModel *model_glue3;

signals:	// �Զ����ź�
	void changedRundataLabel(QString str);	// To Operation
	void changedRundataText(QString str);	// To Operation
	void changedOffsetChart(float x, float y, float A);	// To Operation
	
public slots:	// �����ⲿ�ź�
	void on_changedConfigGlue(bool glue1, bool glue2, bool glue3);			// From Operation
	void on_changedSqlModel(int index);		// From PointDebug

public:
	// дLog�ļ�
	void writRunningLog(QString str);
	
	// ��ȡ��ǰ�¼�
	QString getCurrentTime();


public:		// ��ȡ��λ
	QMap<QString, PointRun> allpoint_pointRun;
	QMap<QString, PointGeneral> allpoint_general;
	QMap<QString, PointGlue> allpoint_glue1;
	QMap<QString, PointGlue> allpoint_glue2;
	QMap<QString, PointGlue> allpoint_glue3;
	
	QMap<QString, PointRun> getAllRunPointInfo();
	QMap<QString, PointGlue> getAllGluePointInfo(int index);
	QMap<QString, PointGeneral> getAllGeneralPointInfo();
	

	float wSpeed;
	float wAcc;
	float wDec;

	void set_speed(float speed, float acc, float dec);
	bool move_point_name(QString pointname, int type, int z_flag);
	bool move_point_name(QString pointname, int z_flag = 0);		// �ƶ�����, by point

};

#endif // WORKFLOW_H