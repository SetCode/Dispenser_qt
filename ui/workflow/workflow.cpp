#include "workflow.h"

Workflow::Workflow(QObject *parent) : QObject(parent)
{
	setIOStatus();
	setConfig();
	setPoint();
	// setCommunication();

	setThread();
}

Workflow::~Workflow()
{
	// ����IO
	close_thread_watch_estop = true;
	close_thread_watch_reset = true;
	close_thread_watch_start = true;
	close_thread_watch_clearNeedle = true;

	// �㽺����
	close_thread_glue_teda = true;

	// �źŲۿ�������
	close_thread_ccd_calib = true;

	thread_pool.waitForDone();
	thread_pool.clear();
	thread_pool.destroyed();
}

void Workflow::setConfig()
{
	// ��1�� ��������
	QFile file("../config/workflow_glue.ini");
	if (!file.exists())
	{
		is_config_gluel = false;
		is_config_glue2 = false;
		is_config_glue3 = false;
	}

	QSettings setting("../config/workflow_glue.ini", QSettings::IniFormat);
	is_config_gluel = setting.value("workflow_glue/is_config_gluel").toBool();
	is_config_glue2 = setting.value("workflow_glue/is_config_glue2").toBool();
	is_config_glue3 = setting.value("workflow_glue/is_config_glue3").toBool();

	// ��2�� ����ƫ������
	distance_ccd_needle_x = setting.value("ccd_needle_diatance/offset_x").toInt() / 1000.0;
	distance_ccd_neddle_y = setting.value("ccd_needle_diatance/offset_y").toInt() / 1000.0;

	distance_ccd_laser_x = setting.value("ccd_laser_diatance/offset_x").toInt() / 1000.0;
	diatance_ccd_laser_y = setting.value("ccd_laser_diatance/offset_x").toInt() / 1000.0;

	distance_laser_needle_x = setting.value("laser_needle_diatance/offset_x").toInt() / 1000.0;
	distance_laser_needle_y = setting.value("laser_needle_diatance/offset_y").toInt() / 1000.0;
	distance_laser_needle_z = setting.value("laser_needle_diatance/offset_z").toInt() / 1000.0;

	calib_offset_x = setting.value("calib_needle_optical/calib_offset_x").toInt() / 1000.0;
	calib_offset_y = setting.value("calib_needle_optical/calib_offset_y").toInt() / 1000.0;
	calib_offset_z = setting.value("calib_needle_attach/calib_offset_z").toInt() / 1000.0;
	file.close();


	// ��2�� ��λ����
	model_general = new QSqlTableModel(this);
	// ʹ�� submit ʱ,���ݿ�Ż����,���������ĸ��Ĵ洢�ڻ�����
	model_general->setEditStrategy(QSqlTableModel::OnManualSubmit);
	model_general->setTable("point_main");
	// ���ð���0����������
	model_general->setSort(0, Qt::AscendingOrder);
	model_general->select();

	model_glue1 = new QSqlTableModel(this);
	model_glue1->setEditStrategy(QSqlTableModel::OnManualSubmit);
	model_glue1->setTable("point_glue1");
	model_glue1->setSort(0, Qt::AscendingOrder);
	model_glue1->select();

	model_glue2 = new QSqlTableModel(this);
	model_glue2->setEditStrategy(QSqlTableModel::OnManualSubmit);
	model_glue2->setTable("point_glue2");
	model_glue2->setSort(0, Qt::AscendingOrder);
	model_glue2->select();

	model_glue3 = new QSqlTableModel(this);
	model_glue3->setEditStrategy(QSqlTableModel::OnManualSubmit);
	model_glue3->setTable("point_glue3");
	model_glue3->setSort(0, Qt::AscendingOrder);
	model_glue3->select();
}

void Workflow::setPoint()
{
	allpoint_pointRun = getAllRunPointInfo();

	vec_ccdGlue_1 = getCCDGluePoint2Vector(1);
	vec_ccdGlue_2 = getCCDGluePoint2Vector(2);
	vec_ccdGlue_3 = getCCDGluePoint2Vector(3);
}

void Workflow::setCommunication()
{
	// TCP CCD
	receivedMsg_ccd = "";
	socket_ccd = new QTcpSocket(this);
	socket_ccd->connectToHost("127.0.0.1", 7777);
	if (!socket_ccd->waitForConnected(10000))
	{
		QMessageBox::about(NULL, "Warning", QStringLiteral("���ӷ�����ʧ��"));
	}
	else
	{
		connect(socket_ccd, &QTcpSocket::readyRead, this, &Workflow::socket_ccd_receive);
	}

	// Serial Laser
	receivedMsg_laser = "";
	serial_laser = new QSerialPort(this);
	serial_laser->setPortName("COM1");
	if (!serial_laser->open(QIODevice::ReadWrite))
	{
		QMessageBox::about(NULL, "Warning", QStringLiteral("����COM1ʧ��"));
	}
	else
	{
		serial_laser->setBaudRate(9600);
		serial_laser->setDataBits(QSerialPort::Data8);
		serial_laser->setParity(QSerialPort::NoParity);
		serial_laser->setStopBits(QSerialPort::OneStop);
		serial_laser->setFlowControl(QSerialPort::NoFlowControl);

		QtConcurrent::run(&thread_pool, [&]() { thread_serialLaserReceive(); });
	}

}

void Workflow::setIOStatus()
{
	if (!(init_card() == 1)) return;

	// ��1�� ��ʼ�����״̬

	// ��2�� �������״̬
}

void Workflow::setThread()
{
	// qDebug() << QThreadPool::globalInstance()->maxThreadCount();
	thread_pool.setMaxThreadCount(20);

	// ��1�� ���� input
	is_estop_ok = false;
	start_thread_watch_estop = true;
	close_thread_watch_estop = false;

	is_reset_ok = false;
	start_thread_watch_reset = true;
	close_thread_watch_reset = false;

	is_start_ok = false;
	start_thread_watch_start = true;
	close_thread_watch_start = false;

	is_clearNeedle_ok = false;
	start_thread_watch_clearNeedle = true;
	close_thread_watch_clearNeedle = false;

	// ��2�� �㽺����  
	is_glue_teda_ok = false;
	start_thread_glue_teda = false;
	close_thread_glue_teda = false;

	// ��3�� �źŲ�����
	is_ccd_calib_ing = false;
	start_thread_ccd_calib = false;
	close_thread_ccd_calib = false;

    future_thread_watch_estop = QtConcurrent::run(&thread_pool, [&]() { thread_watch_estop(); });
	future_thread_watch_reset = QtConcurrent::run(&thread_pool, [&]() { thread_watch_reset(); });
	future_thread_watch_start = QtConcurrent::run(&thread_pool, [&]() { thread_watch_start(); });
	future_thread_watch_clearNeedle = QtConcurrent::run(&thread_pool, [&]() { thread_watch_clearNeedle(); });
	
	future_thread_glue_teda_test = QtConcurrent::run(&thread_pool, [&]() { thread_glue_teda_test(); });
}

// Thread ��ͣ
void Workflow::thread_watch_estop()
{
	if (!(init_card() == 1)) return;

	int step_estop = 0;

	while (close_thread_watch_estop == false)
	{
		switch (step_estop)
		{
		case 0:		// ��⼱ͣ
		{
			if ( (true == start_thread_watch_estop) && (1 == read_in_bit(16)))
			{
				step_estop = 10;
			}
			else
			{
				Sleep(5);
				step_estop = 0;
			}
		}
		break;

		case 10:	// ��ͣ����
		{
			stop_allaxis();
			Sleep(200);
			estop();
			Sleep(200);

			step_estop = 9999;
		}
		break;

		case 8888:	// �߳�: ����ִ�����, �ȴ��´ο�ʼ
		{
			emit changedRundataLabel(QStringLiteral("��ͣ�Ѱ���"));
			emit changedRundataText(QStringLiteral("Thread_watch_estop ��ͣ������������, ���ɿ���ͣ, �����������"));
			writRunningLog(QStringLiteral("Thread_watch_estop ��ͣ������������, ���ɿ���ͣ, �����������"));

			step_estop = 0;
		}
		break;

		case 9999:	// �߳�: �߳��˳�
		{
			emit changedRundataLabel(QStringLiteral("��ͣ�Ѱ���"));
			emit changedRundataText(QStringLiteral("Thread_watch_estop ��ͣ������������, �߳̽��ر�. ���ɿ���ͣ, �����������"));
			writRunningLog(QStringLiteral("Thread_watch_estop ��ͣ������������, �߳̽��ر�. ���ɿ���ͣ, �����������"));

			start_thread_watch_estop = false;
			close_thread_watch_estop = true;
		}
		break;

		default:
			break;
		}
	}
}

// Thread ��λ
void Workflow::thread_watch_reset()
{
	if (!(init_card() == 1)) return;

	int step_reset = 0;

	while (close_thread_watch_reset == false)
	{
		switch (step_reset)
		{
		case 0:		// ��⸴λ�ź�
		{
			if ((true == start_thread_watch_reset) && (0 == read_in_bit(20)))
			{
				Sleep(500);
				step_reset = 10;
			}
			else
			{
				Sleep(5);
				step_reset = 0;
			}
		}
		break;

		case 10:	// ��Ϣ����
		{
			emit changedRundataLabel(QStringLiteral("��λ��ʼ"));
			emit changedRundataText(QStringLiteral("Thread_watch_reset ��λ��ʼ"));
			writRunningLog(QStringLiteral("Thread_watch_reset ��λ��ʼ"));
			step_reset = 20;
		}
		break;

		case 20:	// ��վ��λ
		{
			home_axis(AXISNUM::Z);		
			wait_axis_homeOk(AXISNUM::Z);

			home_axis(AXISNUM::X);
			wait_axis_homeOk(AXISNUM::X);

			home_axis(AXISNUM::Y);
			wait_axis_homeOk(AXISNUM::Y);

			step_reset = 8888;
		}
		break;

		case 8888:	// ������������, �ȴ��´ο�ʼ
		{
			emit changedRundataLabel(QStringLiteral("��λ���, �Ѿ���"));
			emit changedRundataText(QStringLiteral("Thread_watch_reset ��λ������������"));
			writRunningLog(QStringLiteral("Thread_watch_reset ��λ������������"));

			is_reset_ok = true;
			step_reset = 0;
		}
		break;

		case 9999:	// �����쳣����, �̹߳ر�
		{
			emit changedRundataLabel(QStringLiteral("��λʧ��, ����"));
			emit changedRundataText(QStringLiteral("Thread_watch_reset ��λ������������, �̹߳ر�"));
			writRunningLog(QStringLiteral("Thread_watch_reset ��λ�����쳣����, �̹߳ر�"));
			
			start_thread_watch_reset = false;
			close_thread_watch_reset = true;
		}
		break;

		default:
			break;
		}

	}
}

// Thread ���Ӳ���
void Workflow::thread_watch_clearNeedle()
{
	if (!(init_card() == 1)) return;

	int step_clearNeedle = 0;

	while (close_thread_watch_clearNeedle == false)
	{
		switch (step_clearNeedle)
		{
		case 0:		// �ȴ�����
		{
			if ( (true == start_thread_watch_clearNeedle) && (0 == read_in_bit(19)) ) //  
			{
				Sleep(1000);
				is_clearNeedle_ok = false;
				step_clearNeedle = 10;
			}
			else
			{
				Sleep(5);
				step_clearNeedle = 0;
			}
		}
		break;

		case 10:	// �ж��Ƿ����ִ���彺
		{
			if (is_reset_ok == false)
			{
				// ����ϵͳ��ʾ��, ������ǰ�߳�
				MessageBox(NULL, TEXT("δ��λ, �޷�ִ�в���"), TEXT("Warning"), MB_OK);
				
				emit changedRundataText(QStringLiteral("Thread_watch_clearNeedle δ��λ, �޷�ִ�в���"));
				writRunningLog(QStringLiteral("Thread_watch_clearNeedle δ��λ, �޷�ִ�в���"));
				step_clearNeedle = 0;
			}
			else if (card_isMoving() == true)
			{
				// ����ϵͳ��ʾ��, ������ǰ�߳�
				MessageBox(NULL, TEXT("���ƿ���������, �޷�ִ�в���"), TEXT("Warning"), MB_OK);
				
				emit changedRundataText(QStringLiteral("Thread_watch_clearNeedle ���ƿ���������, �޷�ִ�в���"));
				writRunningLog(QStringLiteral("Thread_watch_clearNeedle ���ƿ���������, �޷�ִ�в���"));
				step_clearNeedle = 0;
			}
			else
			{
				// �����̿�ʼ
				emit changedRundataText(QStringLiteral("Thread_watch_clearNeedle ������ʼ"));
				writRunningLog(QStringLiteral("Thread_watch_clearNeedle ������ʼ"));
				step_clearNeedle = 20;
			}
		}

		case 20:	// �彺
		{
			// ��1�� Z�ᵽ��ȫ��
			move_axis_abs(AXISNUM::Z, 0);
			wait_axis_stop(AXISNUM::Z);

			// ��2�� ���彺��ȫ��
			move_point_name("clear_glue_safe");
			wait_allaxis_stop();

			// ��3�� �彺�����ɿ�

			Sleep(1000);

			// ��4�� �ж��彺�����Ƿ��ɿ�
			if (true)
			{
				step_clearNeedle = 30;
			}
			else
			{
				QMessageBox::about(NULL, "Warning", QStringLiteral("���������ɿ�ʧ��"));
				step_clearNeedle = 9999;
			}
		}
		break;

		case 30:
		{
			// ��5�� ���彺��
			move_point_name("clear_glue");
			wait_allaxis_stop();

			// ��6�� �彺���׼н�

			Sleep(500);

			// ��7�� Z�ᵽ��ȫ��
			move_axis_abs(AXISNUM::Z, 0);
			wait_axis_stop(AXISNUM::Z);

			// ��8�� �彺�����ɿ�


			step_clearNeedle = 8888;
		}
		break;

		case 8888:	// ������������, �ȴ��´ο�ʼ
		{
			emit changedRundataText(QStringLiteral("Thread_watch_clearNeedle �彺������������, �ȴ��´ο�ʼ"));
			writRunningLog(QStringLiteral("Thread_watch_clearNeedle �彺������������, �ȴ��´ο�ʼ"));

			is_clearNeedle_ok = true;
			step_clearNeedle = 0;
		}
		break;

		case 9999:	// �����쳣����, �̹߳ر�
		{
			emit changedRundataText(QStringLiteral("Thread_watch_clearNeedle �彺�����쳣����, �̹߳ر�"));
			writRunningLog(QStringLiteral("Thread_watch_clearNeedle �彺�����쳣����, �̹߳ر�"));

			start_thread_watch_clearNeedle = false;
			close_thread_watch_clearNeedle = true;
			step_clearNeedle = 0;
		}
		break;

		default:
			break;
		}
	}
}

// Thread ����
void Workflow::thread_watch_start()
{
	if (!(init_card() == 1)) return;

	int step_start = 0;

	while (close_thread_watch_start == false)
	{
		switch (step_start)
		{
		case 0:		// �ȴ�����
		{
			if ( (true == start_thread_watch_start) && (0 == read_in_bit(17)) )
			{
				if (true == is_reset_ok)
				{
					start_thread_watch_start = false;
					step_start = 10;
				}
				else
				{
					emit changedRundataLabel(QStringLiteral("���ȸ�λ"));
					emit changedRundataText(QStringLiteral("Thread_watch_start δ��λ, �޷�����"));
					writRunningLog(QStringLiteral("Thread_watch_start δ��λ, �޷�����"));

					Sleep(1000);
					step_start = 0;
				}

			}
			else
			{
				Sleep(5);
				step_start = 0;
			}
		}
		break;

		case 10:	// ��Ϣ����
		{
			emit changedRundataLabel(QStringLiteral("������..."));
			emit changedRundataText(QStringLiteral("Thread_watch_start �㽺���̿�ʼ"));
			writRunningLog(QStringLiteral("Thread_watch_start �㽺���̿�ʼ"));

			// ���̵�
			write_out_bit(14, 0);
			write_out_bit(15, 0);
			write_out_bit(16, 1);

			step_start = 20;
		}
		break;

		case 20:	// �㽺̩�￪ʼ
		{
			start_thread_glue_teda = true;
			Sleep(1000);
			step_start = 30;
		}
		break;

		case 30:	// �ȴ��㽺1���
		{
			// �жϵ㽺1�Ƿ����
			if (is_glue_teda_ok == false)
			{
				Sleep(5);
				step_start = 30;
			}
			else
			{
				step_start = 80;
			}
		}
		break;

		case 80:	// ���彺��ȫ��
		{
			// ��1�� ��ԭ��
			move_axis_abs(AXISNUM::Z, 0);
			wait_axis_stop(AXISNUM::Z);

			move_axis_abs(AXISNUM::X, 0);
			move_axis_abs(AXISNUM::Y, 0);

			wait_axis_stop(AXISNUM::X);
			wait_axis_stop(AXISNUM::Y);

			// ��2�� �ȴ��´ο�ʼ
			step_start = 8888;
		}

		case 8888:	// ������������, �ȴ��´ο�ʼ
		{
			// ��1�� ��Ϣ����
			emit changedRundataLabel(QStringLiteral("�������"));
			emit changedRundataText(QStringLiteral("Thread_watch_start ����������������"));
			writRunningLog(QStringLiteral("Thread_watch_start ����������������"));

			// ��2�� ���Ƶ�
			write_out_bit(14, 0);
			write_out_bit(15, 1);
			write_out_bit(16, 0);

			// ��3�� ˢ���̱߳��λ
			start_thread_watch_start = true;
			step_start = 0;
		}
		break;

		case 9999:	// �����쳣����, �߳��˳�
		{
			// ��1�� ��Ϣ����
			emit changedRundataLabel(QStringLiteral("�������"));
			emit changedRundataText(QStringLiteral("Thread_watch_start �����쳣����"));
			writRunningLog(QStringLiteral("Thread_watch_start �����쳣����"));

			// ��2�� �����
			write_out_bit(14, 1);
			write_out_bit(15, 0);
			write_out_bit(16, 0);

			// ��3�� ˢ���̱߳��λ
			close_thread_watch_start = true;
		}
		break;

		default:
			break;
		}
	}
}


// Thread �㽺
void Workflow::thread_glue_1()
{
	if (!(init_card() == 1)) return;

	// ��CCD��ȡ����ƫ�� X, Y, A
	float ccd_offset_x, ccd_offset_y, ccd_offset_A;

	// �����ȡ�ĸ߶�
	float index_laser = 0;
	float laser_z[4];
	float laser_z_average = 0;

	// 4��Բ����Բ������
	float center_result_x[4];
	float center_result_y[4];

	MatrixXf work_matrix;					// CCD��λ��תƽ�ƺ�ľ���
	QVector<CCDGlue>::iterator iter_cmd;	// Vector��ͷβ
	CCDGlue current_cmd;					// ��ǰ��λ
	int     index_current_cmd;				// ��ǰ��λ����
	
	int step_glue1 = 0;

	while (close_thread_glue_1 == false)
	{
		switch (step_glue1)
		{
		case 0:		// �ȴ�����
		{
			if (start_thread_glue_1 == false)
			{
				Sleep(2);
				step_glue1 = 0;
			}
			else
			{
				step_glue1 = 10;
			}
		}
		break;

		case 10:	// ����Ƿ������˸�����
		{
			if (is_config_gluel == false)
			{
				step_glue1 = 7777;
			}
			else
			{
				step_glue1 = 20;
			}
		}
		break;

		case 20:	// �����ٶ�, ģʽ
		{
			// �����������ٶ�, ģʽ
			// set_speed_mode()
			step_glue1 = 30;
		}
		break;

		case 30:	// Z�ᵽ��ȫλ
		{
			move_axis_abs(AXISNUM::Z, 0);
			step_glue1 = 40;
		}
		break;

		case 40:	// ��ǰ�л�ͼ�����
		{
			// ����Ϣ
			step_glue1 = 50;
		}
		break;

		case 50:	// ��ǰ�彺
		{
			// ��1�� ���彺��ȫ��
			move_point_name("xx");
			wait_allaxis_stop();

			// ��2�� �ж��彺����
			if (true)
			{
				QMessageBox::warning(NULL, QStringLiteral("����"), QStringLiteral("�彺����״̬����, ����"));
				step_glue1 = 8888;
			}

			// ��4�� ���彺��
			move_point_name("xx");
			wait_allaxis_stop();

			// ��5�� �彺���׼н�


			// ��6�� ���彺��ȫ��, �˵�λ��ȫ�߶�Ϊ 0
			move_point_name("xx");
			wait_allaxis_stop();

			// ��7�� �彺�����ɿ�

			step_glue1 = 60;
		}
		break;

		case 60:	// �� "�㽺1" ���յ�
		{
			// ���㽺1���յ�
			move_point_name("xx");
			wait_allaxis_stop();

			step_glue1 = 70;
		}
		break;

		case 70:	// ����
		{
			// ����Ϣ����

			step_glue1 = 80;
		}
		break;

		case 80:	// �ȴ���ȡƫ�� ccd_offset_x, ccd_offset_y, ccd_offset_z
		{
			if (receivedMsg_ccd == "" || receivedMsg_ccd.length() < 5)
			{
				step_glue1 = 80;
			}
			else
			{
				if (receivedMsg_ccd.split("]").last() == "-1")
				{
					step_glue1 = 7777;
				}
				else
				{
					ccd_offset_x = receivedMsg_ccd.split("]").at(1).toFloat();
					ccd_offset_y = receivedMsg_ccd.split("]").at(2).toFloat();
					ccd_offset_A = receivedMsg_ccd.split("]").at(3).toFloat();

					receivedMsg_ccd = "";

					step_glue1 = 90;
				}
			}
		}
		break;

		case 90:	// �������ƫ��
		{
			work_matrix = CalCCDGluePoint(vec_ccdGlue_1, ccd_offset_x, ccd_offset_y, ccd_offset_A, org_ccdglue_x[CCDORG::GLUE1ORG], org_ccdglue_y[CCDORG::GLUE1ORG]);
		}
		break;

		case 100:	// ��ʼ�����б�־λ, �����ٶ�ǰհ, ���ò岹�ٶ�
		{
			iter_cmd = vec_ccdGlue_1.begin();
			index_current_cmd = 0;
			
			adt8949_set_speed_pretreat_mode(0, 1);
			set_inp_speed(1);

			step_glue1 = 110;

		}
		break;

		case 110:	// �ж� "����" �� "CCDGlue.type"
		{
			if ( iter_cmd == vec_ccdGlue_1.end())
			{
				step_glue1 = 500;	
			}
			else
			{
				if (vec_ccdGlue_1.at(index_current_cmd).type == QString("null"))
				{
					step_glue1 = 500;
				}
				else if (current_cmd.type == QString("line"))
				{
					current_cmd = vec_ccdGlue_1.at(index_current_cmd);
					step_glue1 = 200;
				}
				else if (current_cmd.type == QString("circle"))
				{
					current_cmd = vec_ccdGlue_1.at(index_current_cmd);
					step_glue1 = 400;
				}
				else
				{
					QMessageBox::warning(NULL, QStringLiteral("����"), QStringLiteral("��λ��������"));
					step_glue1 = 9999;
				}
			}
		}
		break;


		/********************** ֱ�߲岹 **********************/
		case 200:	// ֱ�߲岹
		{
			// �ж� "����"
			if (current_cmd.laser)
			{
				step_glue1 = 300;
			}
			else
			{
				step_glue1 = 210;
			}
		}
		break;

		case 210:	// ����, �ؽ�
		{
			if (current_cmd.open)
			{
				adt8949_set_fifo_io(0, 15, 1, -1);
			}
			else
			{

				adt8949_set_fifo_io(0, 15, 0, -1);
			}

			step_glue1 = 230;
		}
		break;

		/*case 210:	// �Ƿ񿪽�
		{
			if (current_cmd.open)
			{
				if (current_cmd.openAdvance > 0 || current_cmd.openDelay <= 0)
				{
					// ��ǰ����
				}
				else if (current_cmd.openAdvance <= 0 || current_cmd.openDelay > 0)
				{
					// �ͺ󿪽�
				}
				else
				{
					// ��������
					adt8949_set_fifo_io(0, 18, 1, -1);
				}
			}

			step_glue1 = 220;
		}
		break;

		case 220:	// �Ƿ���ǰ�ͺ�ؽ�
		{
			if (current_cmd.closeAdvance > 0 || current_cmd.closeDelay <= 0)
			{
				// ��ǰ�ؽ�
			}
			else if (current_cmd.closeAdvance <= 0 || current_cmd.closeDelay > 0)
			{
				// �ͺ�ؽ�
			}
			step_glue1 = 230;
		}
		break;*/

		case 230:	// ֱ�߲岹
		{
			float move_pos[3];
			move_pos[0] = work_matrix(index_current_cmd, 0) + distance_ccd_needle_x + calib_offset_x;
			move_pos[1] = work_matrix(index_current_cmd, 1) + distance_ccd_neddle_y + calib_offset_y;
			move_pos[2] = work_matrix(index_current_cmd, 2) + distance_ccd_needle_x + calib_offset_z; // +"����߶�";

			// ֱ�߲岹
			move_inp_abs_line3(move_pos[0], move_pos[1], move_pos[2]);

			step_glue1 = 240;
		}
		break;

		/*case 240:	// �Ƿ������ؽ�
		{
			if (current_cmd.close)
			{
				// �ؽ�
				write_out_bit(0, 0);
			}

			step_glue1 = 250;
		}
		break;*/

		case 250:	// �ñ�־λ, ����
		{
			iter_cmd++;
			index_current_cmd++;
			step_glue1 = 110;
		}
		break;


		/********************** ������� **********************/
		case 300:	// �������
		{
			float move_pos[3];

			// move_pos[0] = work_matrix[index_current_cmd, 0] + ;
		}
		break;

		
		/********************** Բ���岹 **********************/
		case 400:	// Բ���岹
		{
			step_glue1 = 410;
		}
		break;

		case 410:	// ����, �ؽ�
		{
			if (current_cmd.open)
			{
				adt8949_set_fifo_io(0, 15, 1, -1);
			}
			else
			{

				adt8949_set_fifo_io(0, 15, 0, -1);
			}

			step_glue1 = 430;
		}
		break;

		/*case 410:	// �Ƿ񿪽�
		{
			if (current_cmd.open)
			{
				if (current_cmd.openAdvance > 0 || current_cmd.openDelay <= 0)
				{
					// ��ǰ����
				}
				else if (current_cmd.openAdvance <= 0 || current_cmd.openDelay > 0)
				{
					// �ͺ󿪽�
				}
				else
				{
					// ��������
					write_out_bit(0, 1);
				}
			}

			step_glue1 = 420;
		}
		break;

		case 420:	// �Ƿ� ��ǰ, �ͺ�ؽ�
		{
			if (current_cmd.closeAdvance > 0 || current_cmd.closeDelay <= 0)
			{
				// ��ǰ�ؽ�
			}
			else if (current_cmd.closeAdvance <= 0 || current_cmd.closeDelay > 0)
			{
				// �ͺ�ؽ�
			}
			step_glue1 = 430;
		}
		break;*/


		case 430:	// Բ���岹
		{
			float move_pos[3];
			move_pos[0] = work_matrix(index_current_cmd, 0) + distance_ccd_needle_x + calib_offset_x;
			move_pos[1] = work_matrix(index_current_cmd, 1) + distance_ccd_neddle_y + calib_offset_y;
			move_pos[2] = work_matrix(index_current_cmd, 2) + distance_ccd_needle_x + calib_offset_z; // +"����߶�";

			// Բ���岹ָ��
			move_inp_abs_line3(move_pos[0], move_pos[1], move_pos[2]);

			step_glue1 = 440;
		}
		break;

		/*case 440:	// �Ƿ������ؽ�
		{
			if (current_cmd.close)
			{
				// �ؽ�
				write_out_bit(0, 0);
			}

			step_glue1 = 450;
		}
		break;*/


		case 450:	// �ñ�־λ, ����
		{
			iter_cmd++;
			index_current_cmd++;

			step_glue1 = 110;
		}
		break;


		/********************** ��λ������Ϻ������� **********************/
		case 500:	// �ر��ٶ�ǰհ
		{
			// �رղ岹������
			// adt8949_set_speed_pretreat_mode(0, 0);

			step_glue1 = 510;
		}
		break;

		case 510:	// �����˶��ٶ�, Z�ᵽ��ȫλ
		{
			// ��1�� �ȴ��岹���
			wait_inp_finish();
			adt8949_set_speed_pretreat_mode(0, 0);

			set_speed_mode(1, 1, 1, 1);

			// ��2�� Z�ᵽ��ȫλ
			move_axis_abs(AXISNUM::Z, 0);
			wait_axis_stop(AXISNUM::Z);

			step_glue1 = 520;
		}
		break;

		case 520:	// ���ü���
		{
			iter_cmd = vec_ccdGlue_1.begin();
			iter_cmd = 0;
			index_current_cmd = 0;

			step_glue1 = 6666;
		}



		case 6666:	// ��������ִ�����, ����0, �ȴ��´δ�������
		{
			// ��1�� ����Ϣ
			emit changedRundataText(QStringLiteral("�㽺1�����"));
			writRunningLog(QStringLiteral("�㽺1�����"));

			start_thread_glue_1 = false;
			step_glue1 = 0;
		}
		break;

		case 7777:	// ���̷�����ִ�����, ����0, �ȴ��´δ�������
		{
			// ��0�� �ٶ�����
			wait_inp_finish();
			adt8949_set_speed_pretreat_mode(0, 0);

			set_speed_mode(1, 1, 1, 1);

			// ��1�� Z�ᵽ��ȫλ
			move_axis_abs(AXISNUM::Z, 0);
			wait_axis_stop(AXISNUM::Z);

			// ��2�� ˢ�����̱�־λ
			iter_cmd = vec_ccdGlue_1.begin();
			index_current_cmd = 0;
			index_laser = 0;

			// ��3�� ����Ϣ
			emit changedRundataText(QStringLiteral("�㽺1����ִ��ʧ��, ��������"));
			writRunningLog(QStringLiteral("�㽺1����ִ��ʧ��, ��������"));

			start_thread_glue_1 = false;
			step_glue1 = 0;
		}
		break;

		case 8888:	// �߳�����ִ�����, �رո��߳�
		{
			// ��1�� Z�ᵽ��ȫλ
			move_axis_abs(AXISNUM::Z, 0);
			wait_axis_stop(AXISNUM::Z);

			// ��2�� ˢ�����̱�־λ
			iter_cmd = vec_ccdGlue_1.begin();
			index_current_cmd = 0;
			index_laser = 0;

			emit changedRundataText(QStringLiteral("�㽺1�߳���������"));
			writRunningLog(QStringLiteral("�㽺1�߳���������"));

			start_thread_glue_1 = false;
			close_thread_glue_1 = true;
			step_glue1 = 0;
		}
		break;

		case 9999:	// �̷߳�����ִ�����, �رո��߳�
		{
			// ��ȫ�˳����߳�
			start_thread_glue_1 = false;
			close_thread_glue_1 = true;

			// ֹͣ������
			stop_allaxis();

			step_glue1 = 0;
		}
		break;

		default:
			break;
		}
	}

}

// Thread �㽺̩��
void Workflow::thread_glue_teda()
{
	if (!(init_card() == 1)) return;

	// ��CCD��ȡ����ƫ�� X, Y, A
	float ccd_offset_x, ccd_offset_y, ccd_offset_A;

	// �����ȡ�ĸ߶�
	float index_laser = 0;
	float laser_z[4];
	float laser_z_average = 0;

	// 4��Բ����Բ������
	float center_result_x[4];
	float center_result_y[4];

	MatrixXf work_matrix;					// CCD��λ��תƽ�ƺ�ľ���
	QVector<CCDGlue>::iterator iter_cmd;	// Vector��ͷβ
	CCDGlue current_cmd;					// ��ǰ��λ
	int     index_current_cmd;				// ��ǰ��λ����

	int step_glue_teda = 0;

	while (close_thread_glue_teda == false)
	{
		switch (step_glue_teda)
		{
		case 0:		// �ȴ�����
		{
			if (start_thread_glue_teda == false)
			{
				Sleep(5);
				step_glue_teda = 0;
			}
			else
			{
				start_thread_glue_teda = false;
				step_glue_teda = 10;
			}
		}
		break;

		case 10:	// ����Ƿ������˸�����
		{
			if (is_config_glue_teda == false)
			{
				step_glue_teda = 7777;
			}
			else
			{
				step_glue_teda = 20;
			}
		}
		break;

		case 20:	// �����ٶ�, ģʽ
		{
			// �����˶��ٶ�
			set_speed_mode(0.1, 5, 5, ADMODE::T);
			
			// ���ò岹�ٶ�
			set_inp_speed_mode(0.1, 5, 5, ADMODE::T);

			step_glue_teda = 30;
		}
		break;

		case 30:	// Z�ᵽ��ȫλ
		{
			move_axis_abs(AXISNUM::Z, 0);
			step_glue_teda = 90;
		}
		break;

		case 40:	// ��ǰ�л�ͼ�����
		{
			// ����Ϣ

			step_glue_teda = 50;
		}
		break;

		case 50:	// ��ǰ�彺
		{
			// ��1�� ���彺��ȫ��
			move_point_name("xx");
			wait_allaxis_stop();

			// ��2�� �ж��彺����
			if (true)
			{
				QMessageBox::warning(NULL, QStringLiteral("����"), QStringLiteral("�彺����״̬����, ����"));
				step_glue_teda = 8888;
			}

			// ��4�� ���彺��
			move_point_name("xx");
			wait_allaxis_stop();

			// ��5�� �彺���׼н�


			// ��6�� ���彺��ȫ��, �˵�λ��ȫ�߶�Ϊ 0
			move_point_name("xx");
			wait_allaxis_stop();

			// ��7�� �彺�����ɿ�

			step_glue_teda = 60;
		}
		break;

		case 60:	// �� "�㽺1" ���յ�
		{
			// ���㽺1���յ�
			move_point_name("xx");
			wait_allaxis_stop();

			step_glue_teda = 70;
		}
		break;

		case 70:	// ����
		{
			// ����Ϣ����

			step_glue_teda = 80;
		}
		break;

		case 80:	// �ȴ���ȡƫ�� ccd_offset_x, ccd_offset_y, ccd_offset_z
		{
			if (receivedMsg_ccd == "" || receivedMsg_ccd.length() < 5)
			{
				step_glue_teda = 80;
			}
			else
			{
				if (receivedMsg_ccd.split("]").last() == "-1")
				{
					step_glue_teda = 7777;
				}
				else
				{
					ccd_offset_x = receivedMsg_ccd.split("]").at(0).toFloat();
					ccd_offset_y = receivedMsg_ccd.split("]").at(1).toFloat();
					ccd_offset_A = receivedMsg_ccd.split("]").at(2).toFloat();

					receivedMsg_ccd = "";

					step_glue_teda = 90;
				}
			}
		}
		break;

		case 90:	// �������ƫ��
		{
			// work_matrix = CalCCDGluePoint(vec_ccdGlue_1, ccd_offset_x, ccd_offset_y, ccd_offset_A, org_ccdglue_x[CCDORG::GLUE1ORG], org_ccdglue_y[CCDORG::GLUE1ORG]);
			work_matrix = CalCCDGluePoint(vec_ccdGlue_1, 0, 0, 0, 0, 0);
			step_glue_teda = 100;
		}
		break;

		case 100:	// ��ʼ�����б�־λ, �����ٶ�ǰհ, ���ò岹�ٶ�
		{
			iter_cmd = vec_ccdGlue_1.begin();
			index_current_cmd = 0;

			adt8949_set_speed_pretreat_mode(0, 1);
			set_inp_speed(1);

			step_glue_teda = 110;

		}
		break;

		case 110:	// �ж� "����" �� "CCDGlue.type"
		{
			if (iter_cmd == vec_ccdGlue_1.end())
			{
				step_glue_teda = 7777;
			}
			else
			{
				current_cmd = vec_ccdGlue_1.at(index_current_cmd);

				if (current_cmd.type == QString("null"))
				{
					step_glue_teda = 7777;
				}
				else if (current_cmd.type == QString("line"))	// ֱ�߲岹
				{
					step_glue_teda = 200;
				}
				else if (current_cmd.type == QString("circle"))	// Բ���岹
				{
					step_glue_teda = 400;
				}
				else if (current_cmd.type == QString("up_abs"))		// Z�����϶�λ
				{
					step_glue_teda = 500;
				}
				else if (current_cmd.type == QString("up_offset"))	// Z������ƫ��
				{
					step_glue_teda = 600;
				}
				else if (current_cmd.type == QString("move_abs"))		// �ᶨλָ��
				{
					step_glue_teda = 700;
				}
				else if (current_cmd.type == QString("move_offset"))	// ��ƫ��ָ��
				{
					step_glue_teda = 800;
				}
				else
				{
					QMessageBox::warning(NULL, QStringLiteral("����"), QStringLiteral("��λ��������"));
					step_glue_teda = 9999;
				}
			}
		}
		break;


		/********************** ֱ�߲岹 **********************/
		case 200:	// ֱ�߲岹
		{
			// �ж� "����"
			if (current_cmd.laser)
			{
				step_glue_teda = 300;
			}
			else
			{
				step_glue_teda = 210;
			}
		}
		break;

		case 210:	// ����, �ؽ�
		{
			if (current_cmd.open)
			{
				adt8949_set_fifo_io(0, 15, 1, -1);
			}
			else
			{

				adt8949_set_fifo_io(0, 15, 0, -1);
			}

			step_glue_teda = 230;
		}
		break;

		case 230:	// ֱ�߲岹
		{
			float move_pos[3];
			move_pos[0] = work_matrix(index_current_cmd, 0) + distance_ccd_needle_x + calib_offset_x;
			move_pos[1] = work_matrix(index_current_cmd, 1) + distance_ccd_neddle_y + calib_offset_y;
			move_pos[2] = work_matrix(index_current_cmd, 2) + distance_ccd_needle_x + calib_offset_z; // +"����߶�";

																									// ֱ�߲岹
			move_inp_abs_line3(move_pos[0], move_pos[1], move_pos[2]);

			step_glue_teda = 240;
		}
		break;

		case 250:	// �ñ�־λ, ����
		{
			iter_cmd++;
			index_current_cmd++;
			step_glue_teda = 110;
		}
		break;



		/********************** ������� **********************/
		case 300:	// �������
		{
			// float move_pos[3];

			// move_pos[0] = work_matrix[index_current_cmd, 0] + ;
		}
		break;



		/********************** Բ���岹 **********************/
		case 400:	// Բ���岹
		{
			step_glue_teda = 410;
		}
		break;

		case 410:	// ����, �ؽ�
		{
			if (current_cmd.open)
			{
				adt8949_set_fifo_io(0, 15, 1, -1);
			}
			else if (current_cmd.close)
			{
				adt8949_set_fifo_io(0, 15, 0, -1);
			}

			step_glue_teda = 430;
		}
		break;

		case 430:	// Բ���岹
		{
			float move_pos[3];
			move_pos[0] = work_matrix(index_current_cmd, 0) + distance_ccd_needle_x + calib_offset_x;
			move_pos[1] = work_matrix(index_current_cmd, 1) + distance_ccd_neddle_y + calib_offset_y;
			move_pos[2] = work_matrix(index_current_cmd, 2) + distance_ccd_needle_x + calib_offset_z; // +"����߶�";

																									// Բ���岹ָ��
			move_inp_abs_line3(move_pos[0], move_pos[1], move_pos[2]);

			step_glue_teda = 440;
		}
		break;

		case 450:	// �ñ�־λ, ����
		{
			iter_cmd++;
			index_current_cmd++;

			step_glue_teda = 110;
		}
		break;



		/********************** Z������, ��λ�˶� **********************/
		case 500:	// Z������, ��λָ��
		{
			step_glue_teda = 510;
		}
		break;

		case 510:	// �ȴ��岹����
		{
			wait_inp_finish();
			step_glue_teda = 520;
		}
		break;

		case 520:	// ����, �ؽ�
		{
			if (current_cmd.open)
			{
				write_out_bit(15, 1);
			}
			else if (current_cmd.close)
			{
				write_out_bit(15, 0);
			}

			step_glue_teda = 530;
		}
		break;

		case 530:	// Z������, ��λ�˶�
		{
			float move_pos_z;
			move_pos_z = work_matrix(index_current_cmd, 2);
			
			move_axis_abs(AXISNUM::Z, move_pos_z, 2, 2, 2);
			wait_axis_stop(AXISNUM::Z);

			step_glue_teda = 540;
		}
		break;

		case 540:	// �ñ�־λ����
		{
			iter_cmd++;
			index_current_cmd++;

			step_glue_teda = 110;
		}
		break;



		/********************** Z������, ƫ��ָ�� **********************/
		case 600:	// Z������, ��λָ��
		{
			step_glue_teda = 610;
		}
		break;

		case 610:	// �ȴ��岹����
		{
			wait_inp_finish();
			step_glue_teda = 620;
		}
		break;

		case 620:	// ����, �ؽ�
		{
			if (current_cmd.open)
			{
				write_out_bit(15, 1);
			}
			else if (current_cmd.close)
			{
				write_out_bit(15, 0);
			}

			step_glue_teda = 630;
		}
		break;

		case 630:	// Z������, ƫ���˶�
		{
			float move_pos_z;
			move_pos_z = work_matrix(index_current_cmd, 2);

			move_axis_offset(AXISNUM::Z, move_pos_z, 2, 2, 2);
			wait_axis_stop(AXISNUM::Z);

			step_glue_teda = 640;
		}
		break;

		case 640:	// �ñ�־λ����
		{
			iter_cmd++;
			index_current_cmd++;

			step_glue_teda = 110;
		}
		break;



		/********************** �ᶨλָ�� **********************/
		case 700:	// Z������, ��λָ��
		{
			step_glue_teda = 710;
		}
		break;

		case 710:	// �ȴ��岹����
		{
			wait_inp_finish();
			step_glue_teda = 720;
		}
		break;

		case 720:	// ����, �ؽ�
		{
			if (current_cmd.open)
			{
				write_out_bit(15, 1);
			}
			else if (current_cmd.close)
			{
				write_out_bit(15, 0);
			}

			step_glue_teda = 630;
		}
		break;

		case 730:	// Z������, ƫ���˶�
		{
			float move_pos[3];
			move_pos[0] = current_cmd.X;
			move_pos[1] = current_cmd.Y;
			move_pos[2] = current_cmd.Z;
			
			move_axis_abs(AXISNUM::X, move_pos[0], 2, 2, 2);
			move_axis_abs(AXISNUM::Y, move_pos[1], 2, 2, 2);
			move_axis_abs(AXISNUM::Z, move_pos[2], 2, 2, 2);
			wait_axis_stop(AXISNUM::X);
			wait_axis_stop(AXISNUM::Y);
			wait_axis_stop(AXISNUM::Z);

			step_glue_teda = 640;
		}
		break;

		case 740:	// �ñ�־λ����
		{
			iter_cmd++;
			index_current_cmd++;

			step_glue_teda = 110;
		}
		break;



		/********************** ��ƫ��ָ�� **********************/
		case 800:	// Z������, ��λָ��
		{
			step_glue_teda = 810;
		}
		break;

		case 810:	// �ȴ��岹����
		{
			wait_inp_finish();
			step_glue_teda = 820;
		}
		break;

		case 820:	// ����, �ؽ�
		{
			if (current_cmd.open)
			{
				write_out_bit(15, 1);
			}
			else if (current_cmd.close)
			{
				write_out_bit(15, 0);
			}

			step_glue_teda = 830;
		}
		break;

		case 830:	// Z������, ƫ���˶�
		{
			float move_pos[3];
			move_pos[0] = current_cmd.X;
			move_pos[1] = current_cmd.Y;
			move_pos[2] = current_cmd.Z;

			move_axis_offset(AXISNUM::X, move_pos[0], 2, 2, 2);
			move_axis_offset(AXISNUM::Y, move_pos[1], 2, 2, 2);
			move_axis_offset(AXISNUM::Z, move_pos[2], 2, 2, 2);
			wait_axis_stop(AXISNUM::X);
			wait_axis_stop(AXISNUM::Y);
			wait_axis_stop(AXISNUM::Z);

			step_glue_teda = 840;
		}
		break;

		case 840:	// �ñ�־λ����
		{
			iter_cmd++;
			index_current_cmd++;

			step_glue_teda = 110;
		}
		break;



		/********************** ��λ������Ϻ������� **********************/
		case 1000:	// �ر��ٶ�ǰհ
		{
			// �رղ岹������
			// adt8949_set_speed_pretreat_mode(0, 0);

			step_glue_teda = 510;
		}
		break;

		case 1010:	// �����˶��ٶ�, Z�ᵽ��ȫλ
		{
			// ��1�� �ȴ��岹���
			wait_inp_finish();
			adt8949_set_speed_pretreat_mode(0, 0);

			set_speed_mode(1, 1, 1, 1);

			// ��2�� Z�ᵽ��ȫλ
			move_axis_abs(AXISNUM::Z, 0);
			wait_axis_stop(AXISNUM::Z);

			step_glue_teda = 520;
		}
		break;

		case 1020:	// ���ü���
		{
			iter_cmd = vec_ccdGlue_1.begin();
			iter_cmd = 0;
			index_current_cmd = 0;

			step_glue_teda = 6666;
		}



		case 6666:	// ��������ִ�����, ����0, �ȴ��´δ�������
		{
			// ��1�� ����Ϣ
			emit changedRundataText(QStringLiteral("�㽺1�����"));
			writRunningLog(QStringLiteral("�㽺1�����"));

			start_thread_glue_1 = false;
			step_glue_teda = 0;
		}
		break;

		case 7777:	// ���̷�����ִ�����, ����0, �ȴ��´δ�������
		{
			// ��0�� �ٶ�����
			wait_inp_finish();
			adt8949_set_speed_pretreat_mode(0, 0);

			set_speed_mode(1, 1, 1, 1);

			// ��1�� Z�ᵽ��ȫλ
			move_axis_abs(AXISNUM::Z, 0);
			wait_axis_stop(AXISNUM::Z);

			// ��2�� ˢ�����̱�־λ
			iter_cmd = vec_ccdGlue_1.begin();
			index_current_cmd = 0;
			index_laser = 0;

			// ��3�� ����Ϣ
			emit changedRundataText(QStringLiteral("�㽺1����ִ��ʧ��, ��������"));
			writRunningLog(QStringLiteral("�㽺1����ִ��ʧ��, ��������"));

			start_thread_glue_1 = false;
			step_glue_teda = 0;
		}
		break;

		case 8888:	// �߳�����ִ�����, �رո��߳�
		{
			// ��1�� Z�ᵽ��ȫλ
			move_axis_abs(AXISNUM::Z, 0);
			wait_axis_stop(AXISNUM::Z);

			// ��2�� ˢ�����̱�־λ
			iter_cmd = vec_ccdGlue_1.begin();
			index_current_cmd = 0;
			index_laser = 0;

			emit changedRundataText(QStringLiteral("�㽺1�߳���������"));
			writRunningLog(QStringLiteral("�㽺1�߳���������"));

			start_thread_glue_1 = false;
			close_thread_glue_1 = true;
			step_glue_teda = 0;
		}
		break;

		case 9999:	// �̷߳�����ִ�����, �رո��߳�
		{
			// ��ȫ�˳����߳�
			start_thread_glue_1 = false;
			close_thread_glue_1 = true;

			// ֹͣ������
			stop_allaxis();

			step_glue_teda = 0;
		}
		break;

		default:
			break;
		}
	}
}

// Thread �㽺̩�����
void Workflow::thread_glue_teda_test()
{
	if (!(init_card() == 1)) return;

	int index_laser = 0;					// Laser����

	MatrixXf work_matrix;					// CCD��λ��תƽ�ƺ�ľ���
	QVector<CCDGlue>::iterator iter_cmd;	// Vector��ͷβ
	CCDGlue current_cmd;					// ��ǰ��λ
	int     index_current_cmd;				// ��ǰ��λ����

	int step_glue_teda = 0;
	while (close_thread_glue_teda == false)
	{
		switch (step_glue_teda)
		{
		case 0:		// �ȴ�����
		{
			if (start_thread_glue_teda == false)
			{
				Sleep(5);
				step_glue_teda = 0;
			}
			else
			{
				// ˢ�±�־λ
				start_thread_glue_teda = false;
				is_glue_teda_ok = false;
				
				step_glue_teda = 10;
			}
		}
		break;

		case 10:	// ����Ƿ������˸�����
		{
			if (is_config_glue_teda == false)
			{
				step_glue_teda = 7777;
			}
			else
			{
				step_glue_teda = 20;
			}
		}
		break;

		case 20:	// �����ٶ�, ģʽ, �����ٶ�ǰհ
		{
			// �����˶��ٶ�
			set_speed_mode(0.1, 20, 20, ADMODE::T);

			// ���ò岹�ٶ�
			set_inp_speed_mode(0.1, 20, 20, ADMODE::T);

			// �����ٶ�ǰհ
			adt8949_set_speed_pretreat_mode(0, 1);

			// ���ò岹�������߶νǵ�����ٶ�
			adt8949_set_speed_constraint(0, 1, 20);	// ���ٶȿ��ԱȲ岹�ٶ��Դ�
			adt8949_set_speed_constraint(0, 2, 20);

			// ���ò岹�������߶νǵ�Ӽ��ٶ�
			adt8949_set_acc_constraint(0, 1, 80000);
			adt8949_set_acc_constraint(0, 2, 80000);

			step_glue_teda = 30;
		}
		break;

		case 30:	// Z�ᵽ��ȫλ
		{
			move_axis_abs(AXISNUM::Z, 0);
			step_glue_teda = 90;
		}
		break;

		case 90:	// �������ƫ��
		{
			// work_matrix = CalCCDGluePoint(vec_ccdGlue_1, ccd_offset_x, ccd_offset_y, ccd_offset_A, org_ccdglue_x[CCDORG::GLUE1ORG], org_ccdglue_y[CCDORG::GLUE1ORG]);
			work_matrix = CalCCDGluePoint(vec_ccdGlue_1, 0, 0, 0, 0, 0);
			cout << work_matrix << endl << endl;
			step_glue_teda = 100;
		}
		break;

		case 100:	// ��ʼ�����б�־λ
		{
			iter_cmd = vec_ccdGlue_1.begin();
			index_current_cmd = 0;

			step_glue_teda = 110;
		}
		break;

		case 110:	// �ж� "����" �� "CCDGlue.type"
		{
			if (iter_cmd == vec_ccdGlue_1.end())
			{
				step_glue_teda = 6666;
			}
			else
			{
				current_cmd = vec_ccdGlue_1.at(index_current_cmd);

				if (current_cmd.type == QString("finish"))
				{
					step_glue_teda = 6666;
				}
				else if (current_cmd.type == QString("line"))		// ֱ�߲岹
				{
					step_glue_teda = 200;
				}
				else if (current_cmd.type == QString("circle"))		// Բ���岹
				{
					step_glue_teda = 300;
				}
				else if (current_cmd.type == QString("laser"))		// �������
				{
					step_glue_teda = 400;
				}
				else if (current_cmd.type == QString("up_abs"))		// Z�ᶨλ�˶�
				{
					step_glue_teda = 500;
				}
				else if (current_cmd.type == QString("up_offset"))	// Z��ƫ���˶�
				{
					step_glue_teda = 600;
				}
				else if (current_cmd.type == QString("move_abs"))	// �ᶨλָ��
				{
					step_glue_teda = 700;
				}
				else if (current_cmd.type == QString("move_offset"))// ��ƫ��ָ��
				{
					step_glue_teda = 800;
				}
				else if (current_cmd.type == QString("open_close"))
				{
					step_glue_teda = 900;
				}
				else if (current_cmd.type == QString("continue"))	// ����ָ��
				{
					step_glue_teda = 1000;
				}
				else if (current_cmd.type == QString("clear_needle"))
				{
					step_glue_teda = 1100;
				}
				else
				{
					MessageBox(NULL, TEXT("��λ��������"), TEXT("����"), MB_OK);
					step_glue_teda = 9999;
				}

				cout << step_glue_teda << endl;
			}
		}
		break;



		/********************** ֱ�߲岹 **********************/
		case 200:	// ����, �ؽ�
		{
			if (current_cmd.open)
			{
				adt8949_set_fifo_io(0, 15, 1, -1);
			}
			else
			{
				adt8949_set_fifo_io(0, 15, 0, -1);
			}

			step_glue_teda = 210;
		}
		break;

		case 210:	// ֱ�߲岹
		{
			float move_pos[3];
			move_pos[0] = work_matrix(index_current_cmd, 0) + distance_ccd_needle_x + calib_offset_x;
			move_pos[1] = work_matrix(index_current_cmd, 1) + distance_ccd_neddle_y + calib_offset_y;
			move_pos[2] = work_matrix(index_current_cmd, 2) + distance_ccd_needle_x + calib_offset_z; 

																									// ֱ�߲岹
			move_inp_abs_line3(move_pos[0], move_pos[1], move_pos[2]);

			step_glue_teda = 220;
		}
		break;

		case 220:	// �ñ�־λ, ����
		{
			iter_cmd++;
			index_current_cmd++;

			step_glue_teda = 110;
		}
		break;


		/********************** Բ���岹 **********************/
		case 300:	// ����, �ؽ�
		{
			if (current_cmd.open)
			{
				adt8949_set_fifo_io(0, 15, 1, -1);
			}
			else if (current_cmd.close)
			{
				adt8949_set_fifo_io(0, 15, 0, -1);
			}
			step_glue_teda = 310;
		}
		break;

		case 310:	// Բ���岹
		{
			float move_pos[3];
			move_pos[0] = work_matrix(index_current_cmd, 0) + distance_ccd_needle_x + calib_offset_x;
			move_pos[1] = work_matrix(index_current_cmd, 1) + distance_ccd_neddle_y + calib_offset_y;

			float center_pos[2];
			CalCCDGlueCenterPoint(center_pos, current_cmd.center_X, current_cmd.center_Y, 0, 0, 0, 0, 0);

			move_inp_abs_arc2(move_pos[0], move_pos[1], center_pos[0], center_pos[1]);

			step_glue_teda = 320;
		}
		break;

		case 320:	// �ñ�־λ, ����
		{
			iter_cmd++;
			index_current_cmd++;

			step_glue_teda = 110;
		}
		break;



		/********************** ������� **********************/
		case 400:	// �ȴ��岹����
		{
			wait_inp_finish();
			step_glue_teda = 410;
		}
		break;

		case 410:	// 
		{
			step_glue_teda = 420;
		}
		break;

		case 420:	// 
		{
			step_glue_teda = 430;
		}
		break;

		case 430:	// �ñ�־λ����
		{
			iter_cmd++;
			index_current_cmd++;

			step_glue_teda = 110;
		}
		break;


		/********************** Z�ᶨλ�˶� **********************/
		case 500:	// �ȴ��岹����
		{
			wait_inp_finish();
			step_glue_teda = 510;
		}
		break;

		case 510:	// ����, �ؽ�
		{
			if (current_cmd.open)
			{
				write_out_bit(15, 1);
			}
			else if (current_cmd.close)
			{
				write_out_bit(15, 0);
			}

			step_glue_teda = 520;
		}
		break;

		case 520:	// Z�ᶨλ�˶�
		{
			float move_pos_z = current_cmd.Z;

			move_axis_abs(AXISNUM::Z, move_pos_z);
			wait_axis_stop(AXISNUM::Z);

			step_glue_teda = 530;
		}
		break;

		case 530:	// �ñ�־λ����
		{
			iter_cmd++;
			index_current_cmd++;

			step_glue_teda = 110;
		}
		break;


		/********************** Z��ƫ��ָ�� **********************/
		case 600:	// �ȴ��岹����
		{
			wait_inp_finish();
			step_glue_teda = 610;
		}
		break;

		case 610:	// ����, �ؽ�
		{
			if (current_cmd.open)
			{
				write_out_bit(15, 1);
			}
			else if (current_cmd.close)
			{
				write_out_bit(15, 0);
			}

			step_glue_teda = 620;
		}
		break;

		case 620:	// Z��ƫ���˶�
		{
			float move_pos_z = current_cmd.Z;

			move_axis_offset(AXISNUM::Z, move_pos_z);
			wait_axis_stop(AXISNUM::Z);

			step_glue_teda = 630;
		}
		break;

		case 630:	// �ñ�־λ����
		{
			iter_cmd++;
			index_current_cmd++;

			step_glue_teda = 110;
		}
		break;


		/********************** �ᶨλָ�� **********************/
		case 700:	// �ȴ��岹����
		{
			wait_inp_finish();
			step_glue_teda = 710;
		}
		break;

		case 710:	// ����, �ؽ�
		{
			if (current_cmd.open)
			{
				write_out_bit(15, 1);
			}
			else if (current_cmd.close)
			{
				write_out_bit(15, 0);
			}

			step_glue_teda = 720;
		}
		break;

		case 720:	// ������˶�
		{
			float move_pos[3];
			move_pos[0] = current_cmd.X;
			move_pos[1] = current_cmd.Y;
			move_pos[2] = current_cmd.Z;

			move_axis_abs(AXISNUM::X, move_pos[0]);
			move_axis_abs(AXISNUM::Y, move_pos[1]);
			move_axis_abs(AXISNUM::Z, move_pos[2]);

			wait_axis_stop(AXISNUM::X);
			wait_axis_stop(AXISNUM::Y);
			wait_axis_stop(AXISNUM::Z);

			step_glue_teda = 730;
		}
		break;

		case 730:	// �ñ�־λ����
		{
			iter_cmd++;
			index_current_cmd++;

			step_glue_teda = 110;
		}
		break;



		/********************** ��ƫ��ָ�� **********************/
		case 800:	// �ȴ��岹����
		{
			wait_inp_finish();
			step_glue_teda = 810;
		}
		break;

		case 810:	// ����, �ؽ�
		{
			if (current_cmd.open)
			{
				write_out_bit(15, 1);
			}
			else if (current_cmd.close)
			{
				write_out_bit(15, 0);
			}

			step_glue_teda = 820;
		}
		break;

		case 820:	// ��ƫ���˶�
		{
			float move_pos[3];
			move_pos[0] = current_cmd.X;
			move_pos[1] = current_cmd.Y;
			move_pos[2] = current_cmd.Z;

			move_axis_offset(AXISNUM::X, move_pos[0]);
			move_axis_offset(AXISNUM::Y, move_pos[1]);
			move_axis_offset(AXISNUM::Z, move_pos[2]);

			wait_axis_stop(AXISNUM::X);
			wait_axis_stop(AXISNUM::Y);
			wait_axis_stop(AXISNUM::Z);

			step_glue_teda = 830;
		}
		break;

		case 830:	// �ñ�־λ����
		{
			iter_cmd++;
			index_current_cmd++;

			step_glue_teda = 110;
		}
		break;



		/********************** ����/�ؽ� **********************/	
		case 900:	// �ȴ��岹����
		{
			wait_inp_finish();
			step_glue_teda = 910;
		}
		break;

		case 910:	// ����, �ؽ�
		{
			if (current_cmd.open)
			{
				write_out_bit(15, 1);
			}
			else if (current_cmd.close)
			{
				write_out_bit(15, 0);
			}

			step_glue_teda = 920;
		}
		break;

		case 920:	// �ñ�־λ����
		{
			iter_cmd++;
			index_current_cmd++;

			step_glue_teda = 110;
		}
		break;



		/********************** ����ָ�� **********************/
		case 1000:	// �ȴ��岹����
		{
			wait_inp_finish();
			step_glue_teda = 1010;
		}
		break;

		case 1010:	// �ñ�־λ����
		{
			iter_cmd++;
			index_current_cmd++;

			step_glue_teda = 110;
		}
		break;


		/********************** �彺ָ�� **********************/
		case 1100:	// �ȴ��岹����
		{
			wait_inp_finish();
			step_glue_teda = 1110;
		}
		break;

		case 1110:	// �彺
		{
			// ��1�� Z�ᵽ��ȫ��
			move_axis_abs(AXISNUM::Z, 0);
			wait_axis_stop(AXISNUM::Z);

			// ��2�� ���彺��ȫ��
			move_point_name("clear_glue_safe");
			wait_allaxis_stop();

			// ��3�� �彺�����ɿ�


			// ��4�� �ж��彺�����Ƿ��ɿ�
			if (true)
			{
				step_glue_teda = 1120;
			}
			else
			{
				QMessageBox::about(NULL, "Warning", QStringLiteral("���������ɿ�ʧ��"));
				step_glue_teda = 9999;
			}
		}
		break;

		case 1120:	
		{
			// ��5�� ���彺��
			move_point_name("clear_glue");
			wait_allaxis_stop();

			// ��6�� �彺���׼н�

			
			// ��7�� Z�ᵽ��ȫ��
			move_axis_abs(AXISNUM::Z, 0);
			wait_axis_stop(AXISNUM::Z);

			// ��8�� �彺�����ɿ�


			step_glue_teda = 1130;
		}
		break;

		case 1130:	// �ñ�־λ����
		{
			iter_cmd++;
			index_current_cmd++;

			step_glue_teda = 110;
		}
		break;


		/********************** ����ָ�� **********************/
		case 1200:	// �ȴ��岹����
		{
			wait_inp_finish();
			step_glue_teda = 1210;
		}
		break;

		case 1210:	// ����
		{
			step_glue_teda = 1220;
		}
		break;

		case 1220:
		{
			step_glue_teda = 1230;
		}
		break;

		case 1230:	// �ñ�־λ����
		{
			iter_cmd++;
			index_current_cmd++;

			step_glue_teda = 110;
		}
		break;




		case 6666:	// ������������, ����0, �ȴ��´δ�������
		{
			// ��1�� �ȴ��岹����
			wait_inp_finish();

			// ��2�� �رղ岹�ٶ�Ԥ����
			adt8949_set_speed_pretreat_mode(0, 0);

			// ��3�� Z�ᵽ��ȫλ
			move_axis_abs(AXISNUM::Z, 0);
			wait_axis_stop(AXISNUM::Z);

			// ��4�� ˢ�����̱�־λ
			iter_cmd = vec_ccdGlue_1.begin();
			index_current_cmd = 0;
			index_laser = 0;

			// ��5�� ��Ϣ����
			emit changedRundataText(QStringLiteral("Thread_glue_teda_test �㽺̩�����������������, �ȴ��´δ�������"));
			writRunningLog(QStringLiteral("Thread_glue_teda_test �㽺̩�����������������, �ȴ��´δ�������"));

			// ��6�� ˢ���̱߳��λ
			start_thread_glue_1 = false;
			is_glue_teda_ok = true;
			step_glue_teda = 0;
		}
		break;

		case 7777:	// �����쳣����, ����0, �ȴ��´δ�������
		{
			// ��1�� �ȴ��岹����
			wait_inp_finish();

			// ��2�� �رղ岹�ٶ�Ԥ����
			adt8949_set_speed_pretreat_mode(0, 0);

			// ��3�� Z�ᵽ��ȫλ
			move_axis_abs(AXISNUM::Z, 0);
			wait_axis_stop(AXISNUM::Z);

			// ��4�� ˢ�����̱�־λ
			iter_cmd = vec_ccdGlue_1.begin();
			index_current_cmd = 0;
			index_laser = 0;

			// ��5�� ��Ϣ����
			emit changedRundataText(QStringLiteral("Thread_glue_teda_test �㽺̩����������쳣����, �ȴ��´ο�ʼ"));
			writRunningLog(QStringLiteral("Thread_glue_teda_test �㽺̩����������쳣����, �ȴ��´ο�ʼ"));

			// ��6�� ˢ���̱߳��λ
			start_thread_glue_1 = false;
			is_glue_teda_ok = true;
			step_glue_teda = 0;
		}
		break;

		case 8888:	// �߳���������, �̹߳ر�
		{
			// ��1�� �ȴ��岹����
			wait_inp_finish();

			// ��2�� �رղ岹�ٶ�Ԥ����
			adt8949_set_speed_pretreat_mode(0, 0);

			// ��3�� Z�ᵽ��ȫλ
			move_axis_abs(AXISNUM::Z, 0);
			wait_axis_stop(AXISNUM::Z);

			// ��4�� ˢ�����̱�־λ
			iter_cmd = vec_ccdGlue_1.begin();
			index_current_cmd = 0;
			index_laser = 0;

			// ��5�� ��Ϣ����
			emit changedRundataText(QStringLiteral("Thread_glue_teda_test �㽺̩����� �߳���������, �̹߳ر�"));
			writRunningLog(QStringLiteral("Thread_glue_teda_test �㽺̩����� �߳���������, �̹߳ر�"));

			// ��6�� ˢ���̱߳��λ
			start_thread_glue_1 = false;
			close_thread_glue_1 = true;
			step_glue_teda = 0;
		}
		break;

		case 9999:	// �߳��쳣����, �̹߳ر�
		{
			// ��1�� �ȴ��岹����
			wait_inp_finish();

			// ��2�� �رղ岹�ٶ�Ԥ����
			adt8949_set_speed_pretreat_mode(0, 0);

			// ��3�� Z�ᵽ��ȫλ
			move_axis_abs(AXISNUM::Z, 0);
			wait_axis_stop(AXISNUM::Z);

			// ��4�� ˢ�����̱�־λ
			iter_cmd = vec_ccdGlue_1.begin();
			index_current_cmd = 0;
			index_laser = 0;

			// ��5�� ��Ϣ����
			emit changedRundataText(QStringLiteral("Thread_glue_teda_test �㽺̩����� �߳��쳣����, �̹߳ر�"));
			writRunningLog(QStringLiteral("Thread_glue_teda_test �㽺̩����� �߳��쳣����, �̹߳ر�"));
			
			// ��6�� ˢ���̱߳��λ
			start_thread_glue_1 = false;
			close_thread_glue_1 = true;
			step_glue_teda = 0;
		}
		break;

		default:
			break;

		}
	}
}



// Thread У�� 
void Workflow::thread_calibNeedle()
{
	if (!(init_card() == 1)) return;

	float x_start_pos, x_end_pos;
	float y_start_pos, y_end_pos;

	int step_calibNeedle = 0;

	while (close_thread_calibNeedle == false)
	{
		switch (step_calibNeedle)
		{
		case 0:		// �ȴ�����
		{
			if (start_thread_calibNeedle == false)
			{
				step_calibNeedle = 0;
			}
			else
			{
				step_calibNeedle = 10;
			}
		}
		break;

		case 10:	// ��У�밲ȫ��
		{
			// �����̿�ʼ
			emit changedRundataText(QStringLiteral("У�뿪ʼ"));
			writRunningLog(QStringLiteral("У�뿪ʼ"));

			// �ƶ���У�밲ȫ��
			move_point_name("calib_needle_safe");
			wait_axis_stop(AXISNUM::X);
			wait_axis_stop(AXISNUM::Y);
			wait_axis_stop(AXISNUM::Z);

			step_calibNeedle = 20;
		}
		break;

		case 20:	// ��У���, X-3, Y-3
		{
			// �ƶ���У�밲ȫ��
			move_point_name("calib_needle");
			wait_axis_stop(AXISNUM::X);
			wait_axis_stop(AXISNUM::Y);
			wait_axis_stop(AXISNUM::Z);

			move_axis_offset(AXISNUM::X, -3);
			move_axis_offset(AXISNUM::Y, -3);
			wait_axis_stop(AXISNUM::X);
			wait_axis_stop(AXISNUM::Y);

			step_calibNeedle = 30;
		}
		break;

		case 30:    // �� X+ ��6mm 
		{
			move_axis_offset(AXISNUM::X, 6);

			step_calibNeedle = 40;
		}
		break;

		case 40:	// �ȴ����� "����X" == 1
		{
			if (1 == read_in_bit(35))	// �˴�д����X
			{
				stop_axis(AXISNUM::X);
				Sleep(100);
				x_start_pos = get_current_pos_axis(AXISNUM::X);

				step_calibNeedle = 50;
			}
			else
			{
				step_calibNeedle = 40;
			}
		}
		break;

		case 50:	// �ȴ����� "����X" == 0
		{
			if (0 == read_in_bit(35))	// �˴�д ����X
			{
				stop_axis(AXISNUM::X);
				Sleep(100);
				x_end_pos = get_current_pos_axis(AXISNUM::X);

				step_calibNeedle = 60;
			}
			else
			{
				step_calibNeedle = 50;
			}
		}
		break;

		case 60:	// ������X�м�, ����ƫ��
		{
			float x_mid_pos = (x_end_pos - x_start_pos) / 2;
			// move_axis_abs(AXISNUM::X, x_mid_pos, wSpeed, wAcc, wDec);
			calib_offset_x = x_mid_pos - allpoint_pointRun["calib_needle"].X;

			step_calibNeedle = 100;
		}
		break;

		case 100:	// �� Y+ ��6mm 
		{
			move_axis_offset(AXISNUM::X, 6);
			step_calibNeedle = 110;
		}
		break;

		case 110:	// �ȴ����� "����Y" == 1
		{
			if (1 == read_in_bit(35))	// �˴�д "����Y"
			{
				stop_axis(AXISNUM::Y);
				Sleep(100);
				y_start_pos = get_current_pos_axis(AXISNUM::Y);

				step_calibNeedle = 120;
			}
			else
			{
				step_calibNeedle = 110;
			}
		}
		break;

		case 120:   // ������ Y+ ��6mm
		{
			move_axis_offset(AXISNUM::X, 6);

			step_calibNeedle = 130;
		}
		break;

		case 130:	// �ȴ����� "����Y" == 0
		{
			if (0 == read_in_bit(35))	// �˴�д ����Y
			{
				stop_axis(AXISNUM::Y);
				Sleep(100);
				y_end_pos = get_current_pos_axis(AXISNUM::Y);

				step_calibNeedle = 140;
			}
			else
			{
				step_calibNeedle = 130;
			}
		}
		break;

		case 140:	// �� "����Y" �м�, ����ƫ��
		{
			float y_mid_pos = (x_end_pos - x_start_pos) / 2;
			// move_axis_abs(AXISNUM::X, x_mid_pos, wSpeed, wAcc, wDec);
			calib_offset_y = y_mid_pos - allpoint_pointRun["calib_needle"].Y;

			step_calibNeedle = 200;
		}
		break;

		case 200:	// Z�ذ�ȫ�߶� 0
		{
			move_axis_abs(AXISNUM::Z, 0.000);
			wait_axis_stop(AXISNUM::Z);

			step_calibNeedle = 300;
		}
		break;

		case 300:
		{
			move_point_name("calib_needle_attach_safe");
			wait_axis_stop(AXISNUM::X);
			wait_axis_stop(AXISNUM::Y);
			wait_axis_stop(AXISNUM::Z);

			step_calibNeedle = 310;
		}
		break;

		case 310:
		{
			move_point_name("calib_needle_attach");
			wait_axis_stop(AXISNUM::X);
			wait_axis_stop(AXISNUM::Y);
			wait_axis_stop(AXISNUM::Z);

			step_calibNeedle = 320;
		}
		break;


		case 320:
		{
			move_axis_offset(AXISNUM::X, calib_offset_x);
			move_axis_offset(AXISNUM::Y, calib_offset_y);
			wait_axis_stop(AXISNUM::X);
			wait_axis_stop(AXISNUM::Y);

			step_calibNeedle = 330;
		}
		break;

		case 330:
		{
			move_axis_offset(AXISNUM::X, calib_offset_x);
			move_axis_offset(AXISNUM::Y, calib_offset_y);
			wait_axis_stop(AXISNUM::X);
			wait_axis_stop(AXISNUM::Y);

			step_calibNeedle = 8888;
		}
		break;


		case 8888:
		{
			emit changedRundataText(QStringLiteral("��ͷУ׼�ѽ���"));
			writRunningLog(QStringLiteral("��ͷУ׼�ѽ���"));

			start_thread_calibNeedle = false;
			close_thread_calibNeedle = true;
			step_calibNeedle = 0;
		}
		break;

		case 9999:
		{
			emit changedRundataText(QStringLiteral("��ͷУ׼ʧ��, �ѽ���"));
			writRunningLog(QStringLiteral("��ͷУ׼ʧ��, �ѽ���"));

			start_thread_calibNeedle = false;
			close_thread_calibNeedle = true;
			step_calibNeedle = 0;
		}
		break;

		default:
			break;
		}
	}
}

// Thread ����
void Workflow::thread_ccd_glue_1()
{
	if (!(init_card() == 1)) return;

	// ��CCD��ȡ����ƫ�� X, Y, A
	float ccd_offset_x, ccd_offset_y, ccd_offset_A;

	// 4��Բ����Բ������
	float center_result_x[4];
	float center_result_y[4];

	MatrixXf work_matrix;					// CCD��λ��תƽ�ƺ�ľ���
	QVector<CCDGlue>::iterator iter_cmd;	// Vector��ͷβ
	CCDGlue current_cmd;					// ��ǰ��λ
	int     index_current_cmd;				// ��ǰ��λ����

	int step_glue1 = 0;

	while (close_thread_ccd_glue_1 == false)
	{
		switch (step_glue1)
		{
		case 0:		// �ȴ�����
		{
			if (start_thread_glue_1 == false)
			{
				Sleep(2);
				step_glue1 = 0;
			}
			else
			{
				step_glue1 = 20;
			}
		}
		break;

		case 20:	// �����ٶ�, ģʽ
		{
			// �����������ٶ�, ģʽ
			// set_speed_mode()

			step_glue1 = 30;
		}
		break;

		case 30:	// Z�ᵽ��ȫλ
		{
			move_axis_abs(AXISNUM::Z, 0);
			step_glue1 = 40;
		}
		break;

		case 40:	// ��ǰ�л�ͼ�����
		{
			// ����Ϣ
			step_glue1 = 60;
		}
		break;

		case 60:	// �� "�㽺1" ���յ�
		{
			// ���㽺1���յ�
			move_point_name("xx");
			wait_allaxis_stop();

			step_glue1 = 70;
		}
		break;

		case 70:	// ����
		{
			// ����Ϣ����

			step_glue1 = 80;
		}
		break;

		case 80:	// �ȴ���ȡƫ�� ccd_offset_x, ccd_offset_y, ccd_offset_z
		{
			if (receivedMsg_ccd == "" || receivedMsg_ccd.length() < 5)
			{
				step_glue1 = 80;
			}
			else
			{
				if (receivedMsg_ccd.split("]").last() == "-1")
				{
					step_glue1 = 7777;
				}
				else
				{
					ccd_offset_x = receivedMsg_ccd.split("]").at(1).toFloat();
					ccd_offset_y = receivedMsg_ccd.split("]").at(2).toFloat();
					ccd_offset_A = receivedMsg_ccd.split("]").at(3).toFloat();

					receivedMsg_ccd = "";

					step_glue1 = 90;
				}
			}
		}
		break;

		case 90:	// �������ƫ��
		{
			work_matrix = CalCCDGluePoint(vec_ccdGlue_1, ccd_offset_x, ccd_offset_y, ccd_offset_A, org_ccdglue_x[CCDORG::GLUE1ORG], org_ccdglue_y[CCDORG::GLUE1ORG]);
		}
		break;

		case 100:	// ��ʼ�����б�־λ, �����ٶ�ǰհ, ���ò岹�ٶ�
		{
			iter_cmd = vec_ccdGlue_1.begin();
			index_current_cmd = 0;

			adt8949_set_speed_pretreat_mode(0, 1);
			set_inp_speed(1);

			step_glue1 = 110;

		}
		break;

		case 110:	// �ж� "����" �� "CCDGlue.type"
		{
			if (iter_cmd == vec_ccdGlue_1.end())
			{
				step_glue1 = 500;
			}
			else
			{
				if (vec_ccdGlue_1.at(index_current_cmd).type == QString("null"))
				{
					step_glue1 = 8888;
				}
				else if (current_cmd.type == QString("line"))
				{
					current_cmd = vec_ccdGlue_1.at(index_current_cmd);
					step_glue1 = 200;
				}
				else if (current_cmd.type == QString("circle"))
				{
					current_cmd = vec_ccdGlue_1.at(index_current_cmd);
					step_glue1 = 400;
				}
				else
				{
					QMessageBox::warning(NULL, QStringLiteral("����"), QStringLiteral("��λ��������"));
					step_glue1 = 9999;
				}
			}
		}
		break;


		/********************** ֱ�߲岹 **********************/
		case 200:	// ֱ�߲岹
		{
			step_glue1 = 230;
		}
		break;

		case 230:	// ֱ�߲岹
		{
			float move_pos[3];
			move_pos[0] = work_matrix(index_current_cmd, 0);
			move_pos[1] = work_matrix(index_current_cmd, 1);
			move_pos[2] = work_matrix(index_current_cmd, 2);
			
			// ֱ�߲岹
			move_inp_abs_line3(move_pos[0], move_pos[1], move_pos[2]);

			step_glue1 = 250;
		}
		break;

		case 250:	// �ñ�־λ, ����
		{
			iter_cmd++;
			index_current_cmd++;
			step_glue1 = 110;
		}
		break;

		/********************** Բ���岹 **********************/
		case 400:	// Բ���岹
		{
			step_glue1 = 430;
		}
		break;

		case 430:	// Բ���岹
		{
			float move_pos[2];
			move_pos[0] = work_matrix(index_current_cmd, 0);
			move_pos[1] = work_matrix(index_current_cmd, 1);

			float center_pos[2];
			CalCCDGlueCenterPoint(center_pos, current_cmd.center_X, current_cmd.center_Y, ccd_offset_x, ccd_offset_y, ccd_offset_A, org_ccdglue_x[CCDORG::GLUE1ORG], org_ccdglue_y[CCDORG::GLUE1ORG]);
			
			move_inp_abs_arc2(move_pos[0], move_pos[1], center_pos[0], center_pos[1]);

			step_glue1 = 450;
		}
		break;

		case 450:	// �ñ�־λ, ����
		{
			iter_cmd++;
			index_current_cmd++;

			step_glue1 = 110;
		}
		break;


		/********************** ��λ������Ϻ������� **********************/
		case 6666:	// ��������ִ�����, ����0, �ȴ��´δ�������
		{
			// ��1�� �ȴ��岹���
			wait_inp_finish();
			adt8949_set_speed_pretreat_mode(0, 0);

			// ��2�� ˢ�����̱�־λ
			iter_cmd = vec_ccdGlue_1.begin();
			index_current_cmd = 0;

			// ��3�� ��Ϣˢ��
			emit changedRundataText(QStringLiteral("CCD�㽺1���������"));
			writRunningLog(QStringLiteral("CCD�㽺1���������"));

			start_thread_glue_1 = false;
			stop_allaxis();
			step_glue1 = 0;
		}
		break;

		case 7777:	// ���̷�����ִ�����, ����0, �ȴ��´δ�������
		{
			// ��1�� �ȴ��岹���
			wait_inp_finish();
			adt8949_set_speed_pretreat_mode(0, 0);

			// ��2�� ˢ�����̱�־λ
			iter_cmd = vec_ccdGlue_1.begin();
			index_current_cmd = 0;

			// ��3�� ����Ϣ
			emit changedRundataText(QStringLiteral("CCD�㽺1��������ִ��ʧ��, ��������"));
			writRunningLog(QStringLiteral("CCD�㽺1����ִ��ʧ��, ��������"));

			start_thread_glue_1 = false;
			stop_allaxis();
			step_glue1 = 0;
		}
		break;

		case 8888:	// �߳�����ִ�����, �رո��߳�
		{
			// ��1�� �ȴ��岹���
			wait_inp_finish();
			adt8949_set_speed_pretreat_mode(0, 0);

			// ��2�� ˢ�����̱�־λ
			iter_cmd = vec_ccdGlue_1.begin();
			index_current_cmd = 0;

			// ��3�� ����Ϣ
			emit changedRundataText(QStringLiteral("CCD�㽺1�����߳�ִ�����"));
			writRunningLog(QStringLiteral("CCD�㽺1�����߳�ִ�����"));

			start_thread_ccd_glue_1 = false;
			close_thread_ccd_glue_1 = true;
			stop_allaxis();
			step_glue1 = 0;
		}
		break;

		case 9999:	// �̷߳�����ִ�����, �رո��߳�
		{
			// ��1�� �ȴ��岹���
			wait_inp_finish();
			adt8949_set_speed_pretreat_mode(0, 0);

			// ��2�� ˢ�����̱�־λ
			iter_cmd = vec_ccdGlue_1.begin();
			index_current_cmd = 0;

			// ��3�� ����Ϣ
			emit changedRundataText(QStringLiteral("CCD�㽺1�����߳�ִ��ʧ��, ��������"));
			writRunningLog(QStringLiteral("CCD�㽺1�����߳�ִ�����"));

			start_thread_glue_1 = false;
			close_thread_glue_1 = true;
			stop_allaxis();
			step_glue1 = 0;
		}
		break;

		default:
			break;
		}
	}
}

// Thread �彺
void Workflow::thread_clearNeedle()
{
	if (!(init_card() == 1)) return;
	if (card_isMoving() == 1)
	{
		MessageBox(NULL, TEXT("���ƿ������˶�, �޷�ִ�в���"), TEXT("Warning"), MB_OK);
		return;
	}
	

	int step_clearNeedle = 0;

	while (close_thread_clearNeedle == false)
	{
		switch (step_clearNeedle)
		{
		case 0:		// �ȴ�����
		{
			if (start_thread_clearNeedle == false)
			{
				step_clearNeedle = 0;
			}
			else
			{
				step_clearNeedle = 5;
			}
		}
		break;

		case 5:		// ��Ϣ����
		{
			// �����̿�ʼ
			emit changedRundataText(QStringLiteral("���뿪ʼ"));
			writRunningLog(QStringLiteral("���뿪ʼ"));
			step_clearNeedle = 10;
		}

		case 10:	// ���뿪ʼ, ̧��ͷ
		{
			move_axis_abs(AXISNUM::Z, 0.000);
			wait_axis_stop(AXISNUM::Z);

			step_clearNeedle = 20;
		}
		break;


		case 20:	// �����밲ȫλ
		{
			move_point_name("xxx");
			wait_allaxis_stop();

			step_clearNeedle = 30;
		}
		break;

		case 30:	// �ж��彺����
		{
			if (true)
			{
				QMessageBox::warning(NULL, QStringLiteral("����"), QStringLiteral("�彺����״̬����, ����"));
				step_clearNeedle = 8888;
			}
			else
			{
				step_clearNeedle = 40;
			}
		}
		break;

		case 40:	// �彺
		{
			// ��1�� ���彺��
			move_point_name("xx");
			wait_allaxis_stop();

			// ��2�� �彺���׼н�


			// ��3�� ���彺��ȫ��, �˵�λ��ȫ�߶�Ϊ 0
			move_axis_abs(AXISNUM::Z, 0.000);
			wait_allaxis_stop();

			// ��4�� �彺�����ɿ�

			step_clearNeedle = 8888;
		}
		break;

		case 8888:	// �߳�����ִ�����, �رո��߳�
		{
			emit changedRundataText(QStringLiteral("�彺��������"));
			writRunningLog(QStringLiteral("�彺��������"));

			start_thread_glue_1 = false;
			close_thread_glue_1 = true;
			step_clearNeedle = 0;
		}
		break;

		case 9999:	// �̷߳�����ִ�����, �رո��߳�
		{
			emit changedRundataText(QStringLiteral("�彺ִ��ʧ��, �ѽ���"));
			writRunningLog(QStringLiteral("�彺ִ��ʧ��, �ѽ���"));

			start_thread_glue_1 = false;
			close_thread_glue_1 = true;
			step_clearNeedle = 0;
		}
		break;

		default:
			break;
		}
	}
}

// Thread CCD�궨
void Workflow::thread_ccd_calib()
{
	int step_nineCalib = 0;

	while (close_thread_ccd_calib == false)
	{
		switch (step_nineCalib)
		{
		case 0:		// �ȴ�����
		{
			if (start_thread_ccd_calib == true) //  
			{
				Sleep(200);
				is_ccd_calib_ing = true;
				step_nineCalib = 10;
			}
			else
			{
				Sleep(5);
				step_nineCalib = 0;
			}
		}
		break;

		case 10:	// ˢ����Ϣ
		{
			// �����̿�ʼ
			emit changedRundataText(QStringLiteral("Thread_nineCalib 9��궨��ʼ"));
			writRunningLog(QStringLiteral("Thread_nineCalib ��궨��ʼ"));
			step_nineCalib = 20;
		}

		case 20:	// �����ٶ�, ģʽ
		{
			// �����˶��ٶ�
			// set_speed_mode(0.1, 20, 20, ADMODE::T);
			step_nineCalib = 30;
		}
		break;

		case 30:	// ���궨���յ�
		{
			step_nineCalib = 8888;
		}
		break;

		case 8888:	// �߳���������, �̹߳ر�
		{
			// ˢ����Ϣ
			emit changedRundataText(QStringLiteral("Thread_ccd_calib CCD�궨��������"));
			writRunningLog(QStringLiteral("Thread_ccd_calib CCD�궨��������"));

			// ˢ���̱߳��λ
			is_ccd_calib_ing = false;
			start_thread_ccd_calib = false;
			close_thread_ccd_calib = true;
			step_nineCalib = 0;
		}
		break;

		default:
			break;
		}
	}
}


// ͨѶ Laser ����
void Workflow::thread_serialLaserReceive()
{
	while (close_thread_serialLaserReceive == false)
	{
		QByteArray readData = serial_laser->readAll();
		if (readData.size() > 5)
		{
			mutex_serial.lock();
			receivedMsg_laser = QString(readData).remove(QChar(2)).remove(QChar(3));
			mutex_serial.unlock();
		}
		readData.clear();
		Sleep(5);
	}
}

// ͨѶ CCD socket
void Workflow::socket_ccd_receive()
{
	QByteArray readData = socket_ccd->read(128);
	if (readData.size() > 5)
	{
		QString str_recerve = QString(readData);
	}
	readData.clear();

}


// ��ȡ���п��˶���λ
QMap<QString, PointRun> Workflow::getAllRunPointInfo()
{
	QMap<QString, PointRun> _allPoint;

	for (int index = 0; index < model_general->rowCount(); index++)
	{
		QString name = model_general->record(index).value("name").toString();
		QString description = model_general->record(index).value("description").toString();
		float X = model_general->record(index).value("X").toString().toFloat();
		float Y = model_general->record(index).value("Y").toString().toFloat();
		float Z = model_general->record(index).value("Z").toString().toFloat();

		PointRun point;
		point.name = name;
		point.description = description;
		point.X = X;
		point.Y = Y;
		point.Z = Z;
		_allPoint.insert(name, point);
	}
	
	for (int index = 0; index < model_glue1->rowCount(); index++)
	{
		QString name = model_glue1->record(index).value("name").toString();
		QString description = model_glue1->record(index).value("description").toString();
		float X = model_glue1->record(index).value("X").toString().toFloat();
		float Y = model_glue1->record(index).value("Y").toString().toFloat();
		float Z = model_glue1->record(index).value("Z").toString().toFloat();

		PointRun point;
		point.name = name;
		point.description = description;
		point.X = X;
		point.Y = Y;
		point.Z = Z;

		_allPoint.insert(name, point);
	}

	for (int index = 0; index < model_glue2->rowCount(); index++)
	{
		QString name = model_glue2->record(index).value("name").toString();
		QString description = model_glue2->record(index).value("description").toString();
		float X = model_glue2->record(index).value("X").toString().toFloat();
		float Y = model_glue2->record(index).value("Y").toString().toFloat();
		float Z = model_glue2->record(index).value("Z").toString().toFloat();

		PointRun point;
		point.name = name;
		point.description = description;
		point.X = X;
		point.Y = Y;
		point.Z = Z;

		_allPoint.insert(name, point);
	}

	for (int index = 0; index < model_glue3->rowCount(); index++)
	{
		QString name = model_glue3->record(index).value("name").toString();
		QString description = model_glue3->record(index).value("description").toString();
		float X = model_glue3->record(index).value("X").toString().toFloat();
		float Y = model_glue3->record(index).value("Y").toString().toFloat();
		float Z = model_glue3->record(index).value("Z").toString().toFloat();

		PointRun point;
		point.name = name;
		point.description = description;
		point.X = X;
		point.Y = Y;
		point.Z = Z;

		_allPoint.insert(name, point);
	}

	return _allPoint;
}

// ��ȡ�㽺��վ��λ, CCDGlue��QVector
QVector<CCDGlue> Workflow::getCCDGluePoint2Vector(int index)
{
	QSqlTableModel *pointmodel = new QSqlTableModel();
	QVector<CCDGlue> _vector_ccdGlue;

	if (1 == index) pointmodel = model_glue1; // QSqlTableModel *pointmodel = model_glue1;
	else if (2 == index) pointmodel = model_glue2;
	else if (3 == index) pointmodel = model_glue3;
	else
	{
		QMessageBox::warning(NULL, "����", QStringLiteral("�������ݿ�ģ�ʹ���"));
		return _vector_ccdGlue;
	}


	for (int index = 0; index < pointmodel->rowCount(); index++)
	{
		QString name = pointmodel->record(index).value("name").toString();
		QString description = pointmodel->record(index).value("description").toString();
		float X = pointmodel->record(index).value("X").toString().toFloat();
		float Y = pointmodel->record(index).value("Y").toString().toFloat();
		float Z = pointmodel->record(index).value("Z").toString().toFloat();
		float center_X = pointmodel->record(index).value("center_X").toString().toFloat();
		float center_Y = pointmodel->record(index).value("center_Y").toString().toFloat();
		float extra_offset_z = pointmodel->record(index).value("extra_offset_z").toString().toFloat();
		bool laser = pointmodel->record(index).value("laser").toBool();
		bool open = pointmodel->record(index).value("open").toBool();
		int openAdvance = pointmodel->record(index).value("openAdvance").toInt();
		int openDelay = pointmodel->record(index).value("openDelay").toInt();
		bool close = pointmodel->record(index).value("close").toBool();
		int closeAdvance = pointmodel->record(index).value("closeAdvance").toInt();
		int closeDelay = pointmodel->record(index).value("closeDelay").toInt();
		QString type = pointmodel->record(index).value("type").toString();

		CCDGlue point; // = { name, description, X, Y, Z, open, openAdvance, openDelay, close, closeAdvance, closeDelay, type };
		point.name = name;
		point.description = description;
		point.X = X;
		point.Y = Y;
		point.Z = Z;
		point.center_X = center_X;
		point.center_Y = center_Y;
		point.extra_offset_z = calib_offset_z;
		point.laser = laser;
		point.open = open;
		point.openAdvance = openAdvance;
		point.openDelay = openDelay;
		point.close = close;
		point.closeAdvance = closeAdvance;
		point.closeDelay = closeDelay;
		point.type = type;

		_vector_ccdGlue.append(point);
	}


	return _vector_ccdGlue;
}

// ����ƽ�ƾ���     offset_x, offset_y
MatrixXf Workflow::CalCCDGluePoint(const QVector<CCDGlue> vector_ccdGlue, const float offset_x, const float offset_y)
{
	// ��1�� ��������
	MatrixXf m2f_ccdGlue(2, 1);			// CCD���� X, Y
	MatrixXf m2f_offset(2, 1);			// ƫ�ƾ��� offset_x, offset_y
	MatrixXf m2f_tmp(2, 1);				// m2f_tmp = m2f_offset + m2f_ccdGlue

	MatrixXf m3f_result_ccdGlue(20, 3);		// ���� 3άpointGlue

											// ��2�� ��ʼ������
	m2f_ccdGlue <<
		0,
		0;

	m2f_offset <<
		offset_x,
		offset_y;

	m2f_tmp <<
		0,
		0;

	// ��3��������
	for (int index = 0; index < vector_ccdGlue.size(); index++)
	{
		// ��ȡCCD����� X��Y
		m2f_ccdGlue <<
			vector_ccdGlue.at(index).X,
			vector_ccdGlue.at(index).Y;

		// ƽ��
		m2f_tmp = m2f_ccdGlue + m2f_offset;

		// ��ȡ����ֵ
		m3f_result_ccdGlue(index, 0) = m2f_tmp(0, 0);
		m3f_result_ccdGlue(index, 1) = m2f_tmp(1, 0);
		m3f_result_ccdGlue(index, 2) = vector_ccdGlue.at(index).Z;
	}

	return  m3f_result_ccdGlue;
}

// ����ƽ����ת����  offset_x, offset_y, offset_angle, org_x, org_y
MatrixXf Workflow::CalCCDGluePoint(const QVector<CCDGlue> vector_ccdGlue, const float offset_x, const float offset_y, const float offset_angle, const float org_x, const float org_y)
{
	MatrixXf m3f_ccdGlue(3, 1);				// CCD����
	MatrixXf m3f_offset(3, 3);				// ƫ�ƾ���

	MatrixXf m3f_angle(3, 3);			    // �ǶȾ���
	MatrixXf m3f_move_left(3, 3);			// ����			
	MatrixXf m3f_move_right(3, 3);			// ����

	MatrixXf m3f_tmp(3, 1);
	MatrixXf m3f_result_ccdGlue(100, 3);

	m3f_ccdGlue <<
		0,
		0,
		1;

	m3f_offset <<
		1, 0, offset_x,
		0, 1, offset_y,
		0, 0, 1;

	m3f_angle <<
		cos(offset_angle), -sin(offset_angle), 0,
		sin(offset_angle), cos(offset_angle), 0,
		0, 0, 1;

	m3f_move_left <<
		1, 0, -org_x,
		0, 1, -org_y,
		0, 0, 1;

	m3f_move_right <<
		1, 0, org_x,
		0, 1, org_x,
		0, 0, 1;

	m3f_tmp <<
		0,
		0,
		1;


	for (int index = 0; index<vector_ccdGlue.size(); index++)
	{
		// ��ȡCCD����� X��Y
		m3f_ccdGlue <<
			vector_ccdGlue.at(index).X,
			vector_ccdGlue.at(index).Y,
			1;

		// ��ԭ����ת
		/*// x = r * cos(A),  y = r * sin(A);
		// _x = r * cos(A + B) = r * cos(A) * cos(B) - r * sin(A) * sin(B) = x * cos(B) - y * sin(B)
		// _y = r * sin(A + B) = r * sin(A) * cos(B) + r * cos(A) * sin(B) = x * sin(B) + y * cos(B)
		// MatrixXf m2f_angle(2, 2);
		// m2f_angle << cos(B), -sin(B),
		//				sin(B), cos(B);*/

		// ��ĳһ����ת
		/*// 1. ���� -> (-org_x, -org_y)
		// 2. ��ת
		//		MatrixXf m3f_angle(3, 3);
		//		m2f_angle << cos(B), -sin(B), 0,
		//					 sin(B),  cos(B), 0,
		//						0,      0,    1;
		// 3. ���� -> (org_x, org_y) */

		/*// ����
		m3f_tmp = m3f_move_left * m3f_ccdGlue;
		cout << m3f_tmp << endl << endl;
		// ��ת
		m3f_tmp = m3f_angle * m3f_tmp;
		cout << m3f_tmp << endl << endl;
		// ����
		m3f_tmp = m3f_move_right * m3f_tmp;
		cout << m3f_tmp << endl << endl;*/

		// ��ת
		m3f_tmp = m3f_move_right * m3f_angle * m3f_move_left * m3f_ccdGlue;

		// ƽ��
		m3f_tmp = m3f_offset * m3f_tmp;

		// ����
		m3f_result_ccdGlue(index, 0) = m3f_tmp(0, 0);
		m3f_result_ccdGlue(index, 1) = m3f_tmp(1, 0);
		m3f_result_ccdGlue(index, 2) = vector_ccdGlue.at(index).Z;
	}

	return  m3f_result_ccdGlue;
}

// ����ƽ����ת��λ
void Workflow::CalCCDGlueCenterPoint(float center_pos[2], const float center_x, const float center_y, const float offset_x, const float offset_y, const float offset_angle, const float org_x, const float org_y)
{
	MatrixXf m3f_ccdGlue(3, 1);				// CCD����
	MatrixXf m3f_offset(3, 3);				// ƫ�ƾ���

	MatrixXf m3f_angle(3, 3);			    // �ǶȾ���
	MatrixXf m3f_move_left(3, 3);			// ����			
	MatrixXf m3f_move_right(3, 3);			// ����

	MatrixXf m3f_tmp(3, 1);

	m3f_ccdGlue <<
		center_x,
		center_y,
		1;

	m3f_offset <<
		1, 0, offset_x,
		0, 1, offset_y,
		0, 0, 1;

	m3f_angle <<
		cos(offset_angle), -sin(offset_angle), 0,
		sin(offset_angle), cos(offset_angle), 0,
		0, 0, 1;

	m3f_move_left <<
		1, 0, -org_x,
		0, 1, -org_y,
		0, 0, 1;

	m3f_move_right <<
		1, 0, org_x,
		0, 1, org_x,
		0, 0, 1;

	m3f_tmp <<
		0,
		0,
		1;

	// ��ת
	m3f_tmp = m3f_move_right * m3f_angle * m3f_move_left * m3f_ccdGlue;

	// ƽ��
	m3f_tmp = m3f_offset * m3f_tmp;

	// ����
	center_pos[0] = m3f_tmp(0, 0);
	center_pos[1] = m3f_tmp(1, 0);
}

// �ƶ�����, by pointname
bool Workflow::move_point_name(QString pointname, int z_flag)
{
	if (!allpoint_pointRun.contains(pointname))
	{
		QMessageBox::warning(NULL, QStringLiteral("����"), QStringLiteral("�Ҳ�����: %1 ����").arg(pointname));
		return false;
	}
	else
	{
		PointRun point = allpoint_pointRun[pointname];

		float x_pos = point.X;
		float y_pos = point.Y;
		float z_pos = point.Z;

		if (ZMOVETYPE::NORMAL == z_flag)
		{
			move_axis_abs(AXISNUM::X, x_pos);
			move_axis_abs(AXISNUM::Y, y_pos);
			move_axis_abs(AXISNUM::Z, z_pos);
			wait_axis_stop(AXISNUM::X);
			wait_axis_stop(AXISNUM::Y);
			wait_axis_stop(AXISNUM::Z);
		}
		else if (ZMOVETYPE::BEFORE == z_flag)
		{
			move_axis_abs(AXISNUM::Z, z_pos);
			wait_axis_stop(AXISNUM::Z);

			move_axis_abs(AXISNUM::X, x_pos);
			move_axis_abs(AXISNUM::Y, y_pos);
			wait_axis_stop(AXISNUM::X);
			wait_axis_stop(AXISNUM::Y);
		}
		else if (ZMOVETYPE::AFTER == z_flag)
		{
			move_axis_abs(AXISNUM::X, x_pos);
			move_axis_abs(AXISNUM::Y, y_pos);
			wait_axis_stop(AXISNUM::X);
			wait_axis_stop(AXISNUM::Y);

			move_axis_abs(AXISNUM::Z, z_pos);
			wait_axis_stop(AXISNUM::Z);
		}
		else
		{
			return false;
		}
	}

	return true;
}


// дlog�ļ�
void Workflow::writRunningLog(QString str)
{
	// ��1�� �����Ϣ��ʽ��
	QDateTime currentDate = QDateTime::currentDateTime();
	QString s_currentDate = currentDate.toString(QStringLiteral("yyyy-MM-dd"));
	QString s_currentTime = getCurrentTime();
	QString s_filepath = QString("../data/log/%1.txt").arg(s_currentDate);
	QString s_write = s_currentTime + "   " + str + "\n";

	// ��2�� ���ļ���д������
	QFile file(s_filepath);
	file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append);
	QTextStream in(&file);
	in << s_write;
	file.close();
}

// ��ȡ��ǰʱ��
QString Workflow::getCurrentTime()
{
	QDateTime currentTime = QDateTime::currentDateTime();
	QString s_currentTime = currentTime.toString(QStringLiteral("yyyy-MM-dd hh:mm:ss"));
	return s_currentTime;
}

// ��ȡ��������
bool Workflow::getLaserData(float laser, int limit_ms)
{
	if (!serial_laser->isOpen())
	{
		return false;
	}

	char data_send[10] = { 0x02, 'M', 'E', 'A', 'S', 'U', 'R', 'E', 0x03 };
	serial_laser->write(data_send);

	int i = 0;
	while (i < limit_ms)
	{
		if (receivedMsg_laser != "")
		{
			float rcv = receivedMsg_laser.toFloat();

			mutex_serial.lock();
			receivedMsg_laser = "";
			mutex_serial.unlock();

			if (rcv > float(45.0) && rcv < float(55.0))
			{
				laser = rcv;
				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			Sleep(1);
			i++;
		}
	}
	return false;
}

// ��ȡ�Ӿ�����
bool Workflow::getVisionData(float offset_x, float offset_y, float offset_z, int limit_ms)
{
	if (!socket_ccd->isOpen())
	{
		return 0;
	}
	
	char data_send[11] = { '0', ',', '0', ',', '0', ',', '0', ',', '1', ';' };
	socket_ccd->write(data_send);

	int i = 0; 
	while (i < limit_ms)
	{
		if (receivedMsg_ccd != "")
		{
			QStringList ldata = receivedMsg_ccd.split("]");
			if (ldata.last() == "1")
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			Sleep(1);
			i++;
		}
	}

	return false;
}



// ���� Operation ���������޸�
void Workflow::on_clicked_check_flowConfig()
{
	// ����ʹ�ö��ļ�, ������Ӱ�����Ч��

	QFile file("../config/workflow_glue.ini");
	if (!file.exists()) 
	{
		QMessageBox::about(NULL, "Warning", QStringLiteral("Workflow ��δ�ҵ� workflow_glue.ini �ļ�"));
		return;
	}
	QSettings setting("../config/workflow_glue.ini", QSettings::IniFormat);

	is_config_gluel = setting.value("workflow_glue/is_config_gluel").toBool();
	is_config_glue2 = setting.value("workflow_glue/is_config_glue2").toBool();
	is_config_glue3 = setting.value("workflow_glue/is_config_glue3").toBool();

	file.close();
}

// ���� Operation ����,ƫ�����޸�
void Workflow::on_clicked_btn_saveDistanceOffset()
{
	// ����ʹ�ö��ļ�, ������Ӱ�����Ч��
	QFile file("../config/workflow_glue.ini");
	if (!file.exists())
	{
		QMessageBox::about(NULL, "Warning", QStringLiteral("Workflow ��δ�ҵ� workflow_glue.ini �ļ�"));
		return;
	}
	QSettings setting("../config/workflow_glue.ini", QSettings::IniFormat);

	distance_ccd_needle_x = setting.value("ccd_needle_diatance/offset_x").toInt() / 1000.0;
	distance_ccd_neddle_y = setting.value("ccd_needle_diatance/offset_y").toInt() / 1000.0;

	distance_ccd_laser_x = setting.value("ccd_laser_diatance/offset_x").toInt() / 1000.0;
	diatance_ccd_laser_y = setting.value("ccd_laser_diatance/offset_x").toInt() / 1000.0;

	distance_laser_needle_x = setting.value("laser_needle_diatance/offset_x").toInt() / 1000.0;
	distance_laser_needle_y = setting.value("laser_needle_diatance/offset_y").toInt() / 1000.0;
	distance_laser_needle_z = setting.value("laser_needle_diatance/offset_z").toInt() / 1000.0;

	calib_offset_x = setting.value("calib_needle_optical/calib_offset_x").toInt() / 1000.0;
	calib_offset_y = setting.value("calib_needle_optical/calib_offset_y").toInt() / 1000.0;
	calib_offset_z = setting.value("calib_needle_attach/calib_offset_z").toInt() / 1000.0;
	file.close();
}

// ���� Operation CCD�궨
void Workflow::on_clicked_btn_ccd_calib()
{
	if (!(init_card() == 1)) return;

	if (is_reset_ok == false)
	{
		QMessageBox::about(NULL, "Warning", QStringLiteral("δ��λ, �޷�����CCD�궨"));
		return;
	}

	if (card_isMoving() == true)
	{
		QMessageBox::about(NULL, "Warning", QStringLiteral("���ƿ���������, �޷�����CCD�궨"));
		return;
	}

	if (is_calibNeedle_ing == true)
	{
		QMessageBox::about(NULL, "Waring", QStringLiteral("���ڱ궨, �����ظ����"));
		return;
	}

	start_thread_ccd_calib = true;
	close_thread_ccd_calib = false;

	future_thread_ccd_calib = QtConcurrent::run(&thread_pool, [&]() { thread_ccd_calib(); });
}

// ���� Operation CCD����
void Workflow::on_clicked_btn_ccd_runEmpty()
{

}

// ���� Operation ���ܲ��㽺
void Workflow::on_clicked_btn_runEmpty()
{

}

// ���� Operation �彺
void Workflow::on_clicked_btn_clearGlue()
{

}

// ���� Operation �Զ��Ž�
void Workflow::on_clicked_btn_dischargeGlue()
{

}

// ���� Operation У��1
void Workflow::on_clicked_btn_needleCalib_1()
{

}

// ���� Operation У��2
void Workflow::on_clicked_btn_needleCalib_2()
{

}

// ���� PointDebug ��λ�����޸�
void Workflow::on_changedSqlModel(int index)
{
	allpoint_pointRun = getAllRunPointInfo();

	if (0 == index)
	{
		model_general->select();
	}
	else if (1 == index)
	{
		model_glue1->select();
		vec_ccdGlue_1 = getCCDGluePoint2Vector(1);	// �㽺���̵�λ��Ϣ, �����Զ�����ָ��
	}
	else if (2 == index)
	{
		model_glue2->select();
		vec_ccdGlue_2 = getCCDGluePoint2Vector(2);
	}
	else if (3 == index)
	{
		model_glue3->select();
		vec_ccdGlue_3 = getCCDGluePoint2Vector(3);
	}
	else return;
}
