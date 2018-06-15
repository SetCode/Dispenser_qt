#include "workflow.h"

Workflow::Workflow(QObject *parent) : QObject(parent)
{
	setIOStatus();
	setConfig();
	setPoint();

	setThread();
}

Workflow::~Workflow()
{
	close_thread_watch_start = true;
	close_thread_watch_reset = true;
	close_thread_watch_stop = true;
	close_thread_watch_estop = true;
	close_thread_exchangeTrays = true;
	close_thread_workflow = true;

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
	// ��1�� ��ȡȫ����λ��Ϣ
	allpoint_general = getAllGeneralPointInfo();
	allpoint_glue1 = getAllGluePointInfo(MODELPOINT::GLUE1);
	allpoint_glue2 = getAllGluePointInfo(MODELPOINT::GLUE2);
	allpoint_glue3 = getAllGluePointInfo(MODELPOINT::GLUE3);
	allpoint_pointRun = getAllRunPointInfo();

	vec_ccdGlue_1 = getCCDGluePoint2Vector(1);
	vec_ccdGlue_2 = getCCDGluePoint2Vector(2);
	vec_ccdGlue_3 = getCCDGluePoint2Vector(3);
}

void Workflow::setSocketMsg()
{
	receivedMsg_ccd = "";
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

	is_start_ok = false;
	start_thread_watch_start = false;
	close_thread_watch_start = false;

	is_reset_ok = false;
	start_thread_watch_reset = false;
	close_thread_watch_reset = false;

	is_stop_ok = false;
	close_thread_watch_stop = false;
	close_thread_watch_stop = false;

	is_estop_ok = false;
	close_thread_watch_estop = false;
	close_thread_watch_estop = false;

	is_workflow_ok = false;
	start_thread_workflow = false;
	close_thread_workflow = false;
	
	is_exchangeTrays_ok = false;
	start_thread_exchangeTrays = false;
	close_thread_exchangeTrays = false;
	
	future_thread_watch_start = QtConcurrent::run(&thread_pool, [&]() { thread_watch_start(); });
	future_thread_watch_reset = QtConcurrent::run(&thread_pool, [&]() { thread_watch_reset(); });
	future_thread_watch_stop  = QtConcurrent::run(&thread_pool, [&]() { thread_watch_stop(); });
	future_thread_watch_estop = QtConcurrent::run(&thread_pool, [&]() { thread_watch_estop(); });
	
	future_thread_workflow = QtConcurrent::run(&thread_pool, [&]() { thread_workflow(); });
	future_thread_exchangeTrays = QtConcurrent::run(&thread_pool, [&]() { thread_exchangeTrays(); });
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
			if (start_thread_watch_start == true || (read_in_bit(20) == 1))
			{
				if (true == is_reset_ok)
				{
					start_thread_watch_start = false;
					step_start = 5;
				}
				else
				{
					emit changedRundataLabel(QStringLiteral("���ȸ�λ"));
					emit changedRundataText(QStringLiteral("δ��λ, �޷�����"));
					writRunningLog(QStringLiteral("δ��λ, �޷�����"));

					Sleep(2);
					step_start = 0;
				}
				
			}
			else
			{
				Sleep(2);
				step_start = 0;
			}
		}
		break;

		case 5:		// ��Ϣ����
		{
			emit changedRundataLabel(QStringLiteral("������..."));
			emit changedRundataText(QStringLiteral("У�뿪ʼ"));
			writRunningLog(QStringLiteral("У�뿪ʼ"));

			step_start = 10;
		}
		break;

		case 10:	// ��ǰ�彺	
		{
			// ��1�� �����ٶ�, ģʽ
			// set_speed_mode()
			
			// ��2�� ���彺��ȫ��
			move_point_name("xx");
			wait_allaxis_stop();

			// ��3�� �ж��彺����
			if (true)
			{

			}

			// ��4�� ���彺��
			move_point_name("xx");
			wait_allaxis_stop();

			// ��5�� �彺���׼н�

			// ��6�� ���彺��ȫ��
			move_point_name("xx");
			wait_allaxis_stop();

			// ��7�� �彺�����ɿ�


			step_start = 20;
		}
		break;

		case 20:	// �㽺1��ʼ
		{
			start_thread_glue_1 = true;
			step_start = 30;
		}
		break;

		case 30:	// �ȴ��㽺1���
		{
			// �жϵ㽺1�Ƿ����
			if (start_thread_glue_1 == false)
			{
				step_start = 30;
			}
			else
			{
				step_start = 40;
			}
		}
		break;

		case 40:	// �㽺2��ʼ
		{
			start_thread_glue_2 = true;
			step_start = 50;
		}
		break;

		case 50:	// �ȴ��㽺2���
		{
			// �ж�"�㽺1"�Ƿ����
			if (start_thread_glue_2 == false)
			{
				step_start = 50;
			}
			else
			{
				step_start = 60;
			}
		}
		break;

		case 60:	// �㽺3��ʼ
		{
			start_thread_glue_3 = true;
			step_start = 70;
		}
		break;

		case 70:	// �ȴ��㽺3���
		{
			if (start_thread_glue_3 == false)
			{
				step_start = 70;
			}
			else
			{
				step_start = 80;
			}
		}
		break;

		case 80:	// ���彺��ȫ��
		{
			// ��1�� ���彺��ȫ��
			move_axis_abs(AXISNUM::Z, 0);
			wait_axis_stop(AXISNUM::Z);

			// ��2�� �ȴ��´ο�ʼ
			step_start = 8888;
		}

		case 8888:	// �߳�: ����ִ�����, �ȴ��´ο�ʼ
		{
			start_thread_watch_start = true;
			step_start = 0;
		}
		break;

		case 9999:	// �߳�: �߳��˳�
		{
			close_thread_watch_start = true;
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
		case 0:	// ��λ��ʼ
			{
				if (start_thread_watch_reset == true || (read_in_bit(20) == 1))
				{
					step_reset = 5;
				}
				else
				{
					Sleep(1);
					step_reset = 0;
				}		
			}
			break;

		case 5:
		{
			// ��Ϣ����
			emit changedRundataLabel(QStringLiteral("��λ��ʼ..."));
			emit changedRundataText(QStringLiteral("��λ��ʼ"));
			writRunningLog(QStringLiteral("��λ��ʼ"));

			step_reset = 10;
		}
		break;

		case 10: // �������
			{


				emit changedRundataLabel(QStringLiteral("��������ź�"));
				emit changedRundataText(QStringLiteral("��������ź�"));
				writRunningLog(QStringLiteral("��������ź�"));
				step_reset = 20;
			}
			break;

		case 20: // ��ʼ�����
			{
				// ��ʼ�����

				emit changedRundataLabel(QStringLiteral("��ʼ�����"));
				emit changedRundataText(QStringLiteral("��ʼ�����"));
				writRunningLog(QStringLiteral("��ʼ�����"));
				step_reset = 30;
					
			}
			break;

		case 30: // �������, ���
			{
				// �������, ���
				emit changedRundataLabel(QStringLiteral("����������"));
				emit changedRundataText(QStringLiteral("����������"));
				writRunningLog(QStringLiteral("����������"));
				
				step_reset = 40;
					
			}
			break;

		case 40: // ��վ��λ
			{ 
				emit changedRundataLabel(QStringLiteral("��վ��λ"));
				emit changedRundataText(QStringLiteral("��վ��λ"));
				writRunningLog(QStringLiteral("��վ��λ"));

				// ��վ��λ
				home_axis(AXISNUM::Z);		
				wait_axis_homeOk(AXISNUM::Z);

				home_axis(AXISNUM::X);
				home_axis(AXISNUM::Y);
				wait_axis_homeOk(AXISNUM::X);
				wait_axis_homeOk(AXISNUM::Y);

				emit changedRundataLabel(QStringLiteral("��վ��λ���"));
				emit changedRundataText(QStringLiteral("��վ��λ���"));
				writRunningLog(QStringLiteral("��վ��λ���"));

				step_reset = 50;
			}
			break;

		case 50: // ��λ���
			{
				emit changedRundataLabel(QStringLiteral("��λ���, �Ѿ���"));
				emit changedRundataText(QStringLiteral("��λ���"));
				writRunningLog(QStringLiteral("��λ���"));

				step_reset = 8888;
					
			}
			break;

		case 8888:	// �߳�: ����ִ�����, �ȴ��´ο�ʼ
			{
				start_thread_watch_reset = false;
				step_reset = 0;
			}
			break;

		case 9999:	// �߳�: �߳��˳�
			{
				close_thread_watch_reset = true;
			}
			break;
			
		default:
			break;
		}
		
	}
}

// Thread ֹͣ
void Workflow::thread_watch_stop()
{
	
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
		case 0:	// ��λ��ʼ
		{
			if (start_thread_watch_reset == true || (read_in_bit(16) == 1))
			{
				step_estop = 5;
			}
			else
			{
				Sleep(1);
				step_estop = 0;
			}
		}
		break;

		case 5:	// ��Ϣ����
		{
			// ��Ϣ����
			emit changedRundataLabel(QStringLiteral("��ͣ�Ѱ���"));
			emit changedRundataText(QStringLiteral("��ͣ�Ѱ���"));
			writRunningLog(QStringLiteral("��ͣ�Ѱ���"));

			step_estop = 10;
		}
		break;

		case 10:
		{
			// ��ͣ
			stop_allaxis();
			estop();
			step_estop = 0;
		}
		break;

		case 8888:	// �߳�: ����ִ�����, �ȴ��´ο�ʼ
		{
			start_thread_watch_estop = false;
			step_estop = 0;
		}
		break;

		case 9999:	// �߳�: �߳��˳�
		{
			close_thread_watch_reset = true;
		}
		break;

		default:
			break;
		}
	}
}

// Thread ��������
void Workflow::thread_workflow()
{
	if (!(init_card() == 1)) return;

	int step_workflow = 0;

	while (close_thread_workflow == false)
	{
		switch (step_workflow)
		{
		case 0: // ���̿�ʼ
		{
			emit changedRundataText(QStringLiteral("�������̿�ʼ"));
			writRunningLog(QStringLiteral("�������̿�ʼ"));

			step_workflow = 10;
			break;
		}

		case 10: // 
		{
			break;
		}

		case 20:
		{
			break;
		}

		case 30:
		{
			break;
		}

		default:
			break;
		}
	}
	
}

// Thread ������
void Workflow::thread_exchangeTrays()
{
	if (!(init_card() == 1)) return;

	int step_tray = 0;

	while (close_thread_exchangeTrays == false)
	{
		switch (step_tray)
		{
		case 0:		// �ȴ�����
			{
				if (start_thread_exchangeTrays == false)
				{
					step_tray = 0;
				}
				else
				{
					step_tray = 5;
				}
			}
			break;

		case 5:		// ��Ϣ����
			{
				// �����̿�ʼ
				emit changedRundataText(QStringLiteral("�����̿�ʼ"));
				writRunningLog(QStringLiteral("�����̿�ʼ"));

				step_tray = 10;
			}

		case 10:	// �����˳�
			{


				if (1 == read_in_bit(33))	// ���̵�λ��Ӧ
				{
					write_out_bit(15, 0);	// �������Ƴ�
					Sleep(2000);

					step_tray = 20;
				}
				else
				{
					QMessageBox::warning(NULL, QStringLiteral("����"), QStringLiteral("���̵�λ��Ӧ�쳣"));

					step_tray = 9999;
				}
			}
			break;

		case 20:	// ��Ӧ�����Ƿ��˳��ɹ�
			{
				if (1 == read_in_bit(34))	// �����˳���λ��Ӧ
				{
					step_tray = 30;
				}
				else
				{
					QMessageBox::warning(NULL, QStringLiteral("����"), QStringLiteral("���̵�λ��Ӧ�쳣"));

					step_tray = 9999;
				}
			}

		case 30:    // �ȴ�����ȡ��, 
			{
				if (1 == read_in_bit(35))	// ����δȡ��
				{
					step_tray = 30;
				}
				else
				{
					Sleep(2000);	// ��ֹȡ�º�, �ַŻ�
					step_tray = 40;
				}
				break;
			}

		case 40:	// �ȴ���������װ��
			{
				if (1 == read_in_bit(35))
				{
					Sleep(3000);	// �ȴ����뿪����
					step_tray = 50;
				}
				else
				{
					step_tray = 40;
				}
			}
			break;

		case 50:	// �������ƹ�ȥ
			{
				write_out_bit(15, 1);
				Sleep(1000);

				step_tray = 8888;
			}
			break;

		case 8888:
			{
				emit changedRundataText(QStringLiteral("�����̽���"));
				writRunningLog(QStringLiteral("�����̽���"));
				start_thread_exchangeTrays = false;
				step_tray = 0;
			}
			break;

		case 9999:
			{
				// ��ȫ�˳����߳�
				close_thread_exchangeTrays = true;

				// ����ֹͣ�ź�
				
			}
			break;

		default:
			break;
		}
	}
}

// Thread �㽺1
void Workflow::thread_glue_1()
{
	if (!(init_card() == 1)) return;

	// ��CCD��ȡ����ƫ�� X, Y, A
	float ccd_offset_x, ccd_offset_y, ccd_offset_A;

	// �����ȡ�ĸ߶�
	float laser_num = 0;
	float laser_z[4];
	float laser_z_average = 0;

	// 4��Բ����Բ������
	float center_result_x[4];
	float center_result_y[4];

	MatrixXf work_matrix;					// CCD��λ��תƽ�ƺ�ľ���
	QVector<CCDGlue>::iterator iter_cmd;	// Vector��ͷβ
	CCDGlue current_cmd;					// ��ǰ��λ
	int     current_cmd_num;				// ��ǰ��λ����
	
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
			current_cmd_num = 0;
			
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
				if (vec_ccdGlue_1.at(current_cmd_num).type == QString("null"))
				{
					step_glue1 = 500;
				}
				else if (current_cmd.type == QString("line"))
				{
					current_cmd = vec_ccdGlue_1.at(current_cmd_num);
					step_glue1 = 200;
				}
				else if (current_cmd.type == QString("circle"))
				{
					current_cmd = vec_ccdGlue_1.at(current_cmd_num);
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
			move_pos[0] = work_matrix(current_cmd_num, 0) + distance_ccd_needle_x + calib_offset_x;
			move_pos[1] = work_matrix(current_cmd_num, 1) + distance_ccd_neddle_y + calib_offset_y;
			move_pos[2] = work_matrix(current_cmd_num, 2) + distance_ccd_needle_x + calib_offset_z; // +"����߶�";

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
			current_cmd_num++;
			step_glue1 = 110;
		}
		break;


		/********************** ������� **********************/
		case 300:	// �������
		{
			float move_pos[3];

			// move_pos[0] = work_matrix[current_cmd_num, 0] + ;
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
			move_pos[0] = work_matrix(current_cmd_num, 0) + distance_ccd_needle_x + calib_offset_x;
			move_pos[1] = work_matrix(current_cmd_num, 1) + distance_ccd_neddle_y + calib_offset_y;
			move_pos[2] = work_matrix(current_cmd_num, 2) + distance_ccd_needle_x + calib_offset_z; // +"����߶�";

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
			current_cmd_num++;

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
			current_cmd_num = 0;

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
			current_cmd_num = 0;
			laser_num = 0;

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
			current_cmd_num = 0;
			laser_num = 0;

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

// Thread �㽺2
void Workflow::thread_glue_2()
{

}

// Thread �㽺3
void Workflow::thread_glue_3()
{

}


void Workflow::thread_glue_teda()
{
	if (!(init_card() == 1)) return;

	// ��CCD��ȡ����ƫ�� X, Y, A
	float ccd_offset_x, ccd_offset_y, ccd_offset_A;

	// �����ȡ�ĸ߶�
	float laser_num = 0;
	float laser_z[4];
	float laser_z_average = 0;

	// 4��Բ����Բ������
	float center_result_x[4];
	float center_result_y[4];

	MatrixXf work_matrix;					// CCD��λ��תƽ�ƺ�ľ���
	QVector<CCDGlue>::iterator iter_cmd;	// Vector��ͷβ
	CCDGlue current_cmd;					// ��ǰ��λ
	int     current_cmd_num;				// ��ǰ��λ����

	int step_glue_teda = 0;

	while (step_glue_teda == false)
	{
		switch (step_glue_teda)
		{
		case 0:		// �ȴ�����
		{
			if (start_thread_glue_teda == false)
			{
				Sleep(1);
				step_glue_teda = 0;
			}
			else
			{
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
			step_glue_teda = 30;
		}
		break;

		case 30:	// Z�ᵽ��ȫλ
		{
			move_axis_abs(AXISNUM::Z, 0);
			step_glue_teda = 40;
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
					ccd_offset_x = receivedMsg_ccd.split("]").at(1).toFloat();
					ccd_offset_y = receivedMsg_ccd.split("]").at(2).toFloat();
					ccd_offset_A = receivedMsg_ccd.split("]").at(3).toFloat();

					receivedMsg_ccd = "";

					step_glue_teda = 90;
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
			current_cmd_num = 0;

			adt8949_set_speed_pretreat_mode(0, 1);
			set_inp_speed(1);

			step_glue_teda = 110;

		}
		break;

		case 110:	// �ж� "����" �� "CCDGlue.type"
		{
			if (iter_cmd == vec_ccdGlue_1.end())
			{
				step_glue_teda = 500;
			}
			else
			{
				if (vec_ccdGlue_1.at(current_cmd_num).type == QString("null"))
				{
					step_glue_teda = 500;
				}
				else if (current_cmd.type == QString("line"))
				{
					current_cmd = vec_ccdGlue_1.at(current_cmd_num);
					step_glue_teda = 200;
				}
				else if (current_cmd.type == QString("circle"))
				{
					current_cmd = vec_ccdGlue_1.at(current_cmd_num);
					step_glue_teda = 400;
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

		step_glue_teda = 220;
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
		step_glue_teda = 230;
		}
		break;*/

		case 230:	// ֱ�߲岹
		{
			float move_pos[3];
			move_pos[0] = work_matrix(current_cmd_num, 0) + distance_ccd_needle_x + calib_offset_x;
			move_pos[1] = work_matrix(current_cmd_num, 1) + distance_ccd_neddle_y + calib_offset_y;
			move_pos[2] = work_matrix(current_cmd_num, 2) + distance_ccd_needle_x + calib_offset_z; // +"����߶�";

																									// ֱ�߲岹
			move_inp_abs_line3(move_pos[0], move_pos[1], move_pos[2]);

			step_glue_teda = 240;
		}
		break;

		/*case 240:	// �Ƿ������ؽ�
		{
		if (current_cmd.close)
		{
		// �ؽ�
		write_out_bit(0, 0);
		}

		step_glue_teda = 250;
		}
		break;*/

		case 250:	// �ñ�־λ, ����
		{
			iter_cmd++;
			current_cmd_num++;
			step_glue_teda = 110;
		}
		break;


		/********************** ������� **********************/
		case 300:	// �������
		{
			float move_pos[3];

			// move_pos[0] = work_matrix[current_cmd_num, 0] + ;
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
			else
			{

				adt8949_set_fifo_io(0, 15, 0, -1);
			}

			step_glue_teda = 430;
		}
		break;

		case 430:	// Բ���岹
		{
			float move_pos[3];
			move_pos[0] = work_matrix(current_cmd_num, 0) + distance_ccd_needle_x + calib_offset_x;
			move_pos[1] = work_matrix(current_cmd_num, 1) + distance_ccd_neddle_y + calib_offset_y;
			move_pos[2] = work_matrix(current_cmd_num, 2) + distance_ccd_needle_x + calib_offset_z; // +"����߶�";

																									// Բ���岹ָ��
			move_inp_abs_line3(move_pos[0], move_pos[1], move_pos[2]);

			step_glue_teda = 440;
		}
		break;

		case 450:	// �ñ�־λ, ����
		{
			iter_cmd++;
			current_cmd_num++;

			step_glue_teda = 110;
		}
		break;


		/********************** ��λ������Ϻ������� **********************/
		case 500:	// �ر��ٶ�ǰհ
		{
			// �رղ岹������
			// adt8949_set_speed_pretreat_mode(0, 0);

			step_glue_teda = 510;
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

			step_glue_teda = 520;
		}
		break;

		case 520:	// ���ü���
		{
			iter_cmd = vec_ccdGlue_1.begin();
			iter_cmd = 0;
			current_cmd_num = 0;

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
			current_cmd_num = 0;
			laser_num = 0;

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
			current_cmd_num = 0;
			laser_num = 0;

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


/*************** �����̲߳���WorkFlow��ִ�� ***************/
// Thread У�� ִ������Ҫ�˳��߳�
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

			move_axis_offset(AXISNUM::X, -3, wSpeed, wAcc, wDec);
			move_axis_offset(AXISNUM::Y, -3, wSpeed, wAcc, wDec);
			wait_axis_stop(AXISNUM::X);
			wait_axis_stop(AXISNUM::Y);

			step_calibNeedle = 30;
		}
		break;

		case 30:    // �� X+ ��6mm 
		{
			move_axis_offset(AXISNUM::X, 6, wSpeed, wAcc, wDec);

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
			move_axis_offset(AXISNUM::X, 6, wSpeed, wAcc, wDec);
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
			move_axis_offset(AXISNUM::X, 6, wSpeed, wAcc, wDec);

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
			move_axis_abs(AXISNUM::Z, 0.000, wSpeed, wAcc, wAcc);
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
			move_axis_offset(AXISNUM::X, calib_offset_x, wSpeed, wAcc, wDec);
			move_axis_offset(AXISNUM::Y, calib_offset_y, wSpeed, wAcc, wDec);
			wait_axis_stop(AXISNUM::X);
			wait_axis_stop(AXISNUM::Y);

			step_calibNeedle = 330;
		}
		break;

		case 330:
		{
			move_axis_offset(AXISNUM::X, calib_offset_x, wSpeed, wAcc, wDec);
			move_axis_offset(AXISNUM::Y, calib_offset_y, wSpeed, wAcc, wDec);
			wait_axis_stop(AXISNUM::X);
			wait_axis_stop(AXISNUM::Y);

			step_calibNeedle = 8888;
		}
		break;


		case 8888:
		{
			emit changedRundataText(QStringLiteral("��ͷУ׼�ѽ���"));
			writRunningLog(QStringLiteral("��ͷУ׼�ѽ���"));

			start_thread_exchangeTrays = false;
			step_calibNeedle = 9999;
		}
		break;

		case 9999:
		{
			// ��ȫ�˳����߳�
			close_thread_exchangeTrays = true;
		}
		break;

		default:
			break;
		}
	}
}

// Thread CCD���ܵ㽺
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
	int     current_cmd_num;				// ��ǰ��λ����

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
			current_cmd_num = 0;

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
				if (vec_ccdGlue_1.at(current_cmd_num).type == QString("null"))
				{
					step_glue1 = 8888;
				}
				else if (current_cmd.type == QString("line"))
				{
					current_cmd = vec_ccdGlue_1.at(current_cmd_num);
					step_glue1 = 200;
				}
				else if (current_cmd.type == QString("circle"))
				{
					current_cmd = vec_ccdGlue_1.at(current_cmd_num);
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
			move_pos[0] = work_matrix(current_cmd_num, 0);
			move_pos[1] = work_matrix(current_cmd_num, 1);
			move_pos[2] = work_matrix(current_cmd_num, 2);
			
			// ֱ�߲岹
			move_inp_abs_line3(move_pos[0], move_pos[1], move_pos[2]);

			step_glue1 = 250;
		}
		break;

		case 250:	// �ñ�־λ, ����
		{
			iter_cmd++;
			current_cmd_num++;
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
			move_pos[0] = work_matrix(current_cmd_num, 0);
			move_pos[1] = work_matrix(current_cmd_num, 1);

			float center_pos[2];
			CalCCDGlueCenterPoint(center_pos, current_cmd.center_X, current_cmd.center_Y, ccd_offset_x, ccd_offset_y, ccd_offset_A, org_ccdglue_x[CCDORG::GLUE1ORG], org_ccdglue_y[CCDORG::GLUE1ORG]);
			
			move_inp_abs_arc2(move_pos[0], move_pos[1], center_pos[0], center_pos[1]);

			step_glue1 = 450;
		}
		break;

		case 450:	// �ñ�־λ, ����
		{
			iter_cmd++;
			current_cmd_num++;

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
			current_cmd_num = 0;

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
			current_cmd_num = 0;

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
			current_cmd_num = 0;

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
			current_cmd_num = 0;

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



// ���� Operation ���������޸�
void Workflow::on_changedConfigGlue(bool glue1, bool glue2, bool glue3)
{
	// ��Ϊ�б�����ļ�, ���Ծ�����ʹ���źŴ���(arg1, arg2, arg3)��.
	/*QFile file("../config/workflow_glue.ini");
	if (!file.exists()) return;
	else
	{
		QSettings setting("../config/workflow_glue.ini", QSettings::IniFormat);
		is_config_gluel = setting.value("workflow_glue/is_config_gluel").toBool();
		is_config_glue2 = setting.value("workflow_glue/is_config_glue2").toBool();
		is_config_glue3 = setting.value("workflow_glue/is_config_glue3").toBool();
	}*/

	// �������źŲ�
	is_config_gluel = glue1;
	is_config_glue2 = glue2;
	is_config_glue3 = glue3;
}

// ���� Operation ƫ�����޸�
void Workflow::on_changedConfigGlueOffset(float offset_x, float offset_y, float offset_z)
{
	calib_offset_x = offset_x;
	calib_offset_y = offset_y;
	calib_offset_z = offset_z;
}


// ���� PointDebug ��λ�����޸�
void Workflow::on_changedSqlModel(int index)
{
	if (0 == index)
	{
		model_general->select();
		allpoint_general = getAllGeneralPointInfo();
	}
	else if (1 == index)
	{
		model_glue1->select();
		allpoint_glue1 = getAllGluePointInfo(1);	// �㽺��λ��Ϣ, �ɲ���
		vec_ccdGlue_1 = getCCDGluePoint2Vector(1);	// �㽺���̵�λ��Ϣ, �����Զ�����ָ��
	}
	else if (2 == index)
	{
		model_glue2->select();
		allpoint_glue2 = getAllGluePointInfo(2);
		vec_ccdGlue_2 = getCCDGluePoint2Vector(2);
	}
	else if (3 == index)
	{
		model_glue3->select();
		allpoint_glue3 = getAllGluePointInfo(3);
		vec_ccdGlue_3 = getCCDGluePoint2Vector(3);
	}
	else return;

	allpoint_pointRun = getAllRunPointInfo();
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


// ��ȡ PointGlue
QMap<QString, PointGlue> Workflow::getAllGluePointInfo(int index)
{
	QSqlTableModel *pointmodel = new QSqlTableModel();
	QMap<QString, PointGlue> _allPoint;

	if (1 == index) pointmodel = model_glue1; // QSqlTableModel *pointmodel = model_glue1;
	else if (2 == index) pointmodel = model_glue2;
	else if (3 == index) pointmodel = model_glue3;
	else
	{
		QMessageBox::warning(NULL, "����", QStringLiteral("�������ݿ�ģ�ʹ���"));
		return _allPoint;
	}

	for (int index = 0; index < pointmodel->rowCount(); index++)
	{
		QString name = pointmodel->record(index).value("name").toString();
		QString description = pointmodel->record(index).value("description").toString();
		float X = pointmodel->record(index).value("X").toString().toFloat();
		float Y = pointmodel->record(index).value("Y").toString().toFloat();
		float Z = pointmodel->record(index).value("Z").toString().toFloat();
		bool open = pointmodel->record(index).value("open").toBool();
		int openAdvance = pointmodel->record(index).value("openAdvance").toInt();
		int openDelay = pointmodel->record(index).value("openDelay").toInt();
		bool close = pointmodel->record(index).value("close").toBool();
		int closeAdvance = pointmodel->record(index).value("closeAdvance").toInt();
		int closeDelay = pointmodel->record(index).value("closeDelay").toInt();
		int type = pointmodel->record(index).value("type").toInt();

		PointGlue point; // = { name, description, X, Y, Z, open, openAdvance, openDelay, close, closeAdvance, closeDelay, type };
		point.name = name;
		point.description = description;
		point.X = X;
		point.Y = Y;
		point.Z = Z;
		point.open = open;
		point.openAdvance = openAdvance;
		point.openDelay = openDelay;
		point.close = close;
		point.closeAdvance = closeAdvance;
		point.closeDelay = closeDelay;
		point.type = type;
		_allPoint.insert(name, point);
	}
	return _allPoint;
}

// ��ȡ PointGeneral
QMap<QString, PointGeneral> Workflow::getAllGeneralPointInfo()
{
	QSqlTableModel *pointmodel = new QSqlTableModel();
	pointmodel = model_general;

	QMap<QString, PointGeneral> _allPoint;
	for (int index = 0; index < pointmodel->rowCount(); index++)
	{
		QString name = pointmodel->record(index).value("name").toString();
		QString description = pointmodel->record(index).value("description").toString();
		float X = pointmodel->record(index).value("X").toString().toFloat();
		float Y = pointmodel->record(index).value("Y").toString().toFloat();
		float Z = pointmodel->record(index).value("Z").toString().toFloat();

		PointGeneral point;
		point.name = name;
		point.description = description;
		point.X = X;
		point.Y = Y;
		point.Z = Z;
		_allPoint.insert(name, point);
	}
	return _allPoint;
}

// ��ȡ RunPoint
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

// ��ȡ CCDGlue �� QVector
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
		bool open = pointmodel->record(index).value("open").toBool();
		int openAdvance = pointmodel->record(index).value("openAdvance").toInt();
		int openDelay = pointmodel->record(index).value("openDelay").toInt();
		bool close = pointmodel->record(index).value("close").toBool();
		int closeAdvance = pointmodel->record(index).value("closeAdvance").toInt();
		int closeDelay = pointmodel->record(index).value("closeDelay").toInt();
		int type = pointmodel->record(index).value("type").toInt();

		CCDGlue point; // = { name, description, X, Y, Z, open, openAdvance, openDelay, close, closeAdvance, closeDelay, type };
		point.name = name;
		point.description = description;
		point.X = X;
		point.Y = Y;
		point.Z = Z;
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
	MatrixXf m3f_result_ccdGlue(22, 3);

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

// ����ƽ����ת����
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


// �޸�ȫ�ֵ��ٶ�
void Workflow::set_speed(float speed, float acc, float dec)
{
	wSpeed = speed;
	wAcc = acc;
	wDec = dec;
}

// �ƶ�����, by pointname, type
bool Workflow::move_point_name(QString pointname, int type, int z_flag)
{
	if (0 == type)
	{
		if (!allpoint_general.contains(pointname))
		{
			QMessageBox::warning(NULL, QStringLiteral("����"), QStringLiteral("�Ҳ�����: %1 ����").arg(pointname));
			return false;
		}
		else
		{
			PointGeneral point = allpoint_general[pointname];

			float x_pos = point.X;
			float y_pos = point.Y;
			float z_pos = point.Z;

			if (0 == z_flag)
			{
				move_axis_abs(AXISNUM::X, x_pos, wSpeed, wAcc, wDec);
				move_axis_abs(AXISNUM::Y, y_pos, wSpeed, wAcc, wDec);
				move_axis_abs(AXISNUM::Z, z_pos, wSpeed, wAcc, wDec);
				wait_axis_stop(AXISNUM::X);
				wait_axis_stop(AXISNUM::Y);
				wait_axis_stop(AXISNUM::Z);
			}
			else if (1 == z_flag)
			{
				move_axis_abs(AXISNUM::Z, z_pos, wSpeed, wAcc, wDec);
				wait_axis_stop(AXISNUM::Z);

				move_axis_abs(AXISNUM::X, x_pos, wSpeed, wAcc, wDec);
				move_axis_abs(AXISNUM::Y, y_pos, wSpeed, wAcc, wDec);
				wait_axis_stop(AXISNUM::X);
				wait_axis_stop(AXISNUM::Y);	
			}
			else if (-1 == z_flag)
			{
				move_axis_abs(AXISNUM::X, x_pos, wSpeed, wAcc, wDec);
				move_axis_abs(AXISNUM::Y, y_pos, wSpeed, wAcc, wDec);
				wait_axis_stop(AXISNUM::X);
				wait_axis_stop(AXISNUM::Y);

				move_axis_abs(AXISNUM::Z, z_pos, wSpeed, wAcc, wDec);
				wait_axis_stop(AXISNUM::Z);
			}
			else return false;
		}
	}
	else if (1 == type)
	{	
		if (!allpoint_glue1.contains(pointname))
		{
			QMessageBox::warning(NULL, QStringLiteral("����"), QStringLiteral("�Ҳ�����: %1 ����").arg(pointname));
			return false;
		}
		else
		{
			PointGlue point = allpoint_glue1[pointname];

			float x_pos = point.X;
			float y_pos = point.Y;
			float z_pos = point.Z;

			if (0 == z_flag)
			{
				move_axis_abs(AXISNUM::X, x_pos, wSpeed, wAcc, wDec);
				move_axis_abs(AXISNUM::Y, y_pos, wSpeed, wAcc, wDec);
				move_axis_abs(AXISNUM::Z, z_pos, wSpeed, wAcc, wDec);
				wait_axis_stop(AXISNUM::X);
				wait_axis_stop(AXISNUM::Y);
				wait_axis_stop(AXISNUM::Z);
			}
			else if (1 == z_flag)
			{
				move_axis_abs(AXISNUM::Z, z_pos, wSpeed, wAcc, wDec);
				wait_axis_stop(AXISNUM::Z);

				move_axis_abs(AXISNUM::X, x_pos, wSpeed, wAcc, wDec);
				move_axis_abs(AXISNUM::Y, y_pos, wSpeed, wAcc, wDec);
				wait_axis_stop(AXISNUM::X);
				wait_axis_stop(AXISNUM::Y);
			}
			else if (-1 == z_flag)
			{
				move_axis_abs(AXISNUM::X, x_pos, wSpeed, wAcc, wDec);
				move_axis_abs(AXISNUM::Y, y_pos, wSpeed, wAcc, wDec);
				wait_axis_stop(AXISNUM::X);
				wait_axis_stop(AXISNUM::Y);

				move_axis_abs(AXISNUM::Z, z_pos, wSpeed, wAcc, wDec);
				wait_axis_stop(AXISNUM::Z);
			}
			else return false;
		}
	}
	else if (2 == type)
	{
		if (!allpoint_glue2.contains(pointname))
		{
			QMessageBox::warning(NULL, QStringLiteral("����"), QStringLiteral("�Ҳ�����: %1 ����").arg(pointname));
			return false;
		}
		else
		{
			PointGlue point = allpoint_glue2[pointname];

			float x_pos = point.X;
			float y_pos = point.Y;
			float z_pos = point.Z;

			if (0 == z_flag)
			{
				move_axis_abs(AXISNUM::X, x_pos, wSpeed, wAcc, wDec);
				move_axis_abs(AXISNUM::Y, y_pos, wSpeed, wAcc, wDec);
				move_axis_abs(AXISNUM::Z, z_pos, wSpeed, wAcc, wDec);
				wait_axis_stop(AXISNUM::X);
				wait_axis_stop(AXISNUM::Y);
				wait_axis_stop(AXISNUM::Z);
			}
			else if (1 == z_flag)
			{
				move_axis_abs(AXISNUM::Z, z_pos, wSpeed, wAcc, wDec);
				wait_axis_stop(AXISNUM::Z);

				move_axis_abs(AXISNUM::X, x_pos, wSpeed, wAcc, wDec);
				move_axis_abs(AXISNUM::Y, y_pos, wSpeed, wAcc, wDec);
				wait_axis_stop(AXISNUM::X);
				wait_axis_stop(AXISNUM::Y);
			}
			else if (-1 == z_flag)
			{
				move_axis_abs(AXISNUM::X, x_pos, wSpeed, wAcc, wDec);
				move_axis_abs(AXISNUM::Y, y_pos, wSpeed, wAcc, wDec);
				wait_axis_stop(AXISNUM::X);
				wait_axis_stop(AXISNUM::Y);

				move_axis_abs(AXISNUM::Z, z_pos, wSpeed, wAcc, wDec);
				wait_axis_stop(AXISNUM::Z);
			}
			else return false;
		}
	}
	else if (3 == type)
	{
		if (!allpoint_glue3.contains(pointname))
		{
			QMessageBox::warning(NULL, QStringLiteral("����"), QStringLiteral("�Ҳ�����: %1 ����").arg(pointname));
			return false;
		}
		else
		{
			PointGlue point = allpoint_glue3[pointname];

			float x_pos = point.X;
			float y_pos = point.Y;
			float z_pos = point.Z;

			if (0 == z_flag)
			{
				move_axis_abs(AXISNUM::X, x_pos, wSpeed, wAcc, wDec);
				move_axis_abs(AXISNUM::Y, y_pos, wSpeed, wAcc, wDec);
				move_axis_abs(AXISNUM::Z, z_pos, wSpeed, wAcc, wDec);
				wait_axis_stop(AXISNUM::X);
				wait_axis_stop(AXISNUM::Y);
				wait_axis_stop(AXISNUM::Z);
			}
			else if (1 == z_flag)
			{
				move_axis_abs(AXISNUM::Z, z_pos, wSpeed, wAcc, wDec);
				wait_axis_stop(AXISNUM::Z);

				move_axis_abs(AXISNUM::X, x_pos, wSpeed, wAcc, wDec);
				move_axis_abs(AXISNUM::Y, y_pos, wSpeed, wAcc, wDec);
				wait_axis_stop(AXISNUM::X);
				wait_axis_stop(AXISNUM::Y);
			}
			else if (-1 == z_flag)
			{
				move_axis_abs(AXISNUM::X, x_pos, wSpeed, wAcc, wDec);
				move_axis_abs(AXISNUM::Y, y_pos, wSpeed, wAcc, wDec);
				wait_axis_stop(AXISNUM::X);
				wait_axis_stop(AXISNUM::Y);

				move_axis_abs(AXISNUM::Z, z_pos, wSpeed, wAcc, wDec);
				wait_axis_stop(AXISNUM::Z);
			}
			else return false;
		}
	}
	else return false;

	return true;
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
			move_axis_abs(AXISNUM::X, x_pos, wSpeed, wAcc, wDec);
			move_axis_abs(AXISNUM::Y, y_pos, wSpeed, wAcc, wDec);
			move_axis_abs(AXISNUM::Z, z_pos, wSpeed, wAcc, wDec);
			wait_axis_stop(AXISNUM::X);
			wait_axis_stop(AXISNUM::Y);
			wait_axis_stop(AXISNUM::Z);
		}
		else if (ZMOVETYPE::BEFORE == z_flag)
		{
			move_axis_abs(AXISNUM::Z, z_pos, wSpeed, wAcc, wDec);
			wait_axis_stop(AXISNUM::Z);

			move_axis_abs(AXISNUM::X, x_pos, wSpeed, wAcc, wDec);
			move_axis_abs(AXISNUM::Y, y_pos, wSpeed, wAcc, wDec);
			wait_axis_stop(AXISNUM::X);
			wait_axis_stop(AXISNUM::Y);
		}
		else if (ZMOVETYPE::AFTER == z_flag)
		{
			move_axis_abs(AXISNUM::X, x_pos, wSpeed, wAcc, wDec);
			move_axis_abs(AXISNUM::Y, y_pos, wSpeed, wAcc, wDec);
			wait_axis_stop(AXISNUM::X);
			wait_axis_stop(AXISNUM::Y);

			move_axis_abs(AXISNUM::Z, z_pos, wSpeed, wAcc, wDec);
			wait_axis_stop(AXISNUM::Z);
		}
		else return false;
	}

	return true;
}
