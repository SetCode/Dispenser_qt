#include "workflow.h"

Workflow::Workflow(QObject *parent) : QObject(parent)
{
	setIOStatus();
	setConfig();
	setPoint();

	setThread();
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
	else
	{
		QSettings setting("../config/workflow_glue.ini", QSettings::IniFormat);
		is_config_gluel = setting.value("workflow_glue/is_config_gluel").toBool();
		is_config_glue2 = setting.value("workflow_glue/is_config_glue2").toBool();
		is_config_glue3 = setting.value("workflow_glue/is_config_glue3").toBool();
	}

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

	thread_pool.setMaxThreadCount(6);

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
		case 0:
		{
			if (start_thread_watch_start == true || (read_in_bit(20) == 1))
			{
				step_start = 5;
			}
			else
			{
				Sleep(1);
				step_start = 0;
			}
		}
		break;

		case 5:		// ��Ϣ����
		{
			step_start = 10;
		}
		break;

		case 10:
		{
		}
		break;

		case 8888:	// �߳�: ����ִ�����, �ȴ��´ο�ʼ
		{
			start_thread_watch_start = false;
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
			glue_offset_x = x_mid_pos - allpoint_pointRun["calib_needle"].X;

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
			glue_offset_y = y_mid_pos - allpoint_pointRun["calib_needle"].Y;

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
			move_axis_offset(AXISNUM::X, glue_offset_x, wSpeed, wAcc, wDec);
			move_axis_offset(AXISNUM::Y, glue_offset_y, wSpeed, wAcc, wDec);
			wait_axis_stop(AXISNUM::X);
			wait_axis_stop(AXISNUM::Y);

			step_calibNeedle = 330;
		}
		break;

		case 330:
		{
			move_axis_offset(AXISNUM::X, glue_offset_x, wSpeed, wAcc, wDec);
			move_axis_offset(AXISNUM::Y, glue_offset_y, wSpeed, wAcc, wDec);
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

// Thread �㽺1
void Workflow::thread_glue_1()
{

}

// Thread �㽺2
void Workflow::thread_glue_2()
{

}

// Thread �㽺3
void Workflow::thread_glue_3()
{

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
		allpoint_glue1 = getAllGluePointInfo(1);
	}
	else if (2 == index)
	{
		model_glue2->select();
		allpoint_glue2 = getAllGluePointInfo(2);
	}
	else if (3 == index)
	{
		model_glue3->select();
		allpoint_glue3 = getAllGluePointInfo(3);
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
