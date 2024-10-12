#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_MainWindow.h"
#include <QString>
#include <memory>

// LSL API
#include <lsl_cpp.h>

#include <_7i_sdk.h>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindowClass; };
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public:
    static void __stdcall onEyesGazeCallback(const _7I_EYE_DATA_EX* eyesData, void* data);

private:
    void sendLSLData(const _7I_EYE_DATA_EX* eyesData);

private slots:
    void onLinkDevice();

private:
    std::unique_ptr<lsl::stream_outlet> m_outlet;
    Ui::MainWindowClass *ui;
};
