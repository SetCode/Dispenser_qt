﻿#include "qio.h"

QInput::QInput(QWidget *parent) : QWidget(parent)
{
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground, true);

    m_status = 0;
    m_width  = 20;
    m_height = 20;

    setFixedSize(m_width, m_height);
}

void QInput::setSize(int w, int h)
{
    m_width = w;
    m_height = h;
    setFixedSize(w, h);
    this->update();
}

void QInput::setStatus(int status)
{
    if (m_status != status)
    {
        m_status = status;
        this->update();
    }
}

int QInput::getStatus()
{
    return m_status;
}

void QInput::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    // 反走样
    painter.setRenderHint(QPainter::Antialiasing, true);
    // 中心坐标移到正中心
    painter.translate( width()/2, height()/2 );
    painter.setPen(Qt::transparent);

    // 背景填充
    // painter.fillRect(-width(), -height(), width()*2, height()*2, Qt::white);

    int radius = 8;

    // 外边框
    QLinearGradient lg1(0, -radius, 0, radius);
    lg1.setColorAt(0, QColor(240, 240, 240));
    lg1.setColorAt(1, QColor(150, 150, 150));
    painter.setBrush(lg1);
    painter.drawEllipse(-radius, -radius, radius << 1, radius << 1);

    // 内边框
    radius -= 2;
    QLinearGradient lg2(0, -radius, 0, radius);
    lg2.setColorAt(0, QColor(120, 120, 120));
    lg2.setColorAt(1, QColor(160, 160, 160));
    painter.setBrush(lg2);
    painter.drawEllipse(-radius, -radius, radius << 1, radius << 1);

    if(m_status == 1)
    {
        // 设置 绿色
        radius -= 1;
        QRadialGradient rg(0, 0, radius);
        rg.setColorAt(0,   QColor(0, 245, 0));
        rg.setColorAt(0.6, QColor(0, 210, 0));
        rg.setColorAt(1,   QColor(0, 166, 0));
        painter.setBrush(rg);
        painter.drawEllipse(-radius, -radius, radius << 1, radius << 1);
    }
}


QInputLabel::QInputLabel(QWidget *parent) : QWidget(parent)
{
    label_name  = new QLabel();
    label_name->setAlignment(Qt::AlignCenter);
    label_name->setStyleSheet(QStringLiteral("background-color: rgb(230, 230, 230); border-radius:5px;"));

    label_state = new QInput();

    QHBoxLayout *layout = new QHBoxLayout();
    layout->setMargin(0);
    layout->setSpacing(1);
    layout->addWidget(label_name);
    layout->addWidget(label_state);

    setLayout(layout);
    setFixedSize(150, 20);
}

QInputLabel::QInputLabel(const QString &name, const int &state, QWidget *parent) : QWidget(parent)
{
    label_name  = new QLabel();
    label_name->setAlignment(Qt::AlignCenter);
    label_name->setStyleSheet(QStringLiteral("background-color: rgb(230, 230, 230); border-radius: 5px;"));

    label_state = new QInput();

    QHBoxLayout *layout = new QHBoxLayout();
    layout->setMargin(0);
    layout->setSpacing(1);
    layout->addWidget(label_name);
    layout->addWidget(label_state);

    setLayout(layout);
    setFixedSize(150, 20);

    label_name->setText(name);
    label_state->setStatus(state);
}

void QInputLabel::setName(const QString &name)
{
    if (label_name->text() != name) label_name->setText(name);
}

void QInputLabel::setStatus(int state)
{
    label_state->setStatus(state);
}

QString QInputLabel::getName()
{
    return label_name->text();
}

int QInputLabel::getStatus()
{
    return label_state->getStatus();
}


QOutputButton::QOutputButton(QWidget *parent) : QWidget(parent)
{
    setFixedSize(150, 20);
    m_status = 0;
    btn_output = new QPushButton(this);
    btn_output->setFixedSize(this->size());
    btn_output->setStyleSheet(QStringLiteral("background-color: #D1D1D1; border-radius: 5px;"));

    connect(btn_output, &QPushButton::clicked, this, &QOutputButton::on_btn_output);
}

QOutputButton::QOutputButton(const QString &name, int state, QWidget *parent) : QWidget(parent)
{
    setFixedSize(150, 20);
    m_status = 0;
    btn_output = new QPushButton(this);
    btn_output->setFixedSize(this->size());
    btn_output->setStyleSheet(QStringLiteral("background-color: #D1D1D1; border-radius: 5px;"));

    connect(btn_output, &QPushButton::clicked, this, &QOutputButton::on_btn_output);

    setName(name);
    setStatus(state);
}

void QOutputButton::setName(const QString &name)
{
    if (btn_output->text() != name) btn_output->setText(name);
}

void QOutputButton::setStatus(int state)
{
    if (m_status == state) return;
    else
    {
        m_status = state;
        if( m_status <= 0) btn_output->setStyleSheet(QStringLiteral("background-color: #D1D1D1; border-radius: 5px;"));
        else if(m_status == 1) btn_output->setStyleSheet(QStringLiteral("background-color: #5CACEE; color: #eff0f1; border-radius: 5px;"));
    }
}

QString QOutputButton::getName()
{
    return btn_output->text();
}

int QOutputButton::getStatus()
{
    return m_status;
}

void QOutputButton::on_btn_output()
{
    emit qclicked();
}
