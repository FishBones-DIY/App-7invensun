#include "MainWindow.h"

#include <QPushButton>
#include <QMessagebox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindowClass())
{
    ui->setupUi(this);

    setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

    connect(ui->linkButton, &QPushButton::clicked, this, &MainWindow::onLinkDevice);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onLinkDevice()
{
    if (m_outlet)
    {
        try 
        {
            _7i_stop();
            _7i_device_disconnect();
            m_outlet.reset();
        } 
        catch (std::exception& e) 
        {
            QMessageBox::critical(this, "Error", (std::string("Stop SDK error : ") += e.what()).c_str(), QMessageBox::Ok);
            return;
        }
        
        ui->linkButton->setText("Start Device");
    }
    else
    {
        bool bConnect = false;
        bool bStart = false;
        try
        {
            if (_7i_device_connect(nullptr, nullptr) != 0)
                throw std::runtime_error("Connect SDK error");
            bConnect = true;
            
            _7i_set_gaze_callback(onEyesGazeCallback, this);

            if (_7i_start(QCoreApplication::applicationDirPath().toUtf8().data()) != 0)
                throw std::runtime_error("Start camera error");
            bStart = true;

            lsl::stream_info info("7invensun", "Gaze", 14, 0, lsl::cf_float32, "7invensun_Gaze");
            lsl::xml_element channels = info.desc().append_child("channels");
            channels.append_child("channel")
                .append_child_value("label", "Screen_X_left")
                .append_child_value("eye", "left")
                .append_child_value("type", "ScreenX");
            channels.append_child("channel")
                .append_child_value("label", "Screen_Y_left")
                .append_child_value("eye", "left")
                .append_child_value("type", "ScreenY");
            channels.append_child("channel")
                .append_child_value("label", "Pupil_X_left")
                .append_child_value("eye", "left")
                .append_child_value("type", "PupilX");
            channels.append_child("channel")
                .append_child_value("label", "Pupil_Y_left")
                .append_child_value("eye", "left")
                .append_child_value("type", "PupilY");
            channels.append_child("channel")
                .append_child_value("label", "Diameter_left")
                .append_child_value("eye", "left")
                .append_child_value("type", "Diameter");
            channels.append_child("channel")
                .append_child_value("label", "Distance_left")
                .append_child_value("eye", "left")
                .append_child_value("type", "Distance");
            channels.append_child("channel")
                .append_child_value("label", "Confidence_left")
                .append_child_value("eye", "left")
                .append_child_value("type", "Confidence");

            channels.append_child("channel")
                .append_child_value("label", "Screen_X_right")
                .append_child_value("eye", "right")
                .append_child_value("type", "ScreenX");
            channels.append_child("channel")
                .append_child_value("label", "Screen_Y_right")
                .append_child_value("eye", "right")
                .append_child_value("type", "ScreenY");
            channels.append_child("channel")
                .append_child_value("label", "Pupil_X_right")
                .append_child_value("eye", "right")
                .append_child_value("type", "PupilX");
            channels.append_child("channel")
                .append_child_value("label", "Pupil_Y_right")
                .append_child_value("eye", "right")
                .append_child_value("type", "PupilY");
            channels.append_child("channel")
                .append_child_value("label", "Diameter_right")
                .append_child_value("eye", "right")
                .append_child_value("type", "Diameter");
            channels.append_child("channel")
                .append_child_value("label", "Distance_right")
                .append_child_value("eye", "right")
                .append_child_value("type", "Distance");
            channels.append_child("channel")
                .append_child_value("label", "Confidence_right")
                .append_child_value("eye", "right")
                .append_child_value("type", "Confidence");

            info.desc().append_child("acquisition")
                .append_child_value("manufacturer", "7invensun");

            m_outlet.reset(new lsl::stream_outlet(info));
        }
        catch (std::exception& e)
        {
            QMessageBox::critical(this, "Error", (std::string("Could not initialize the 7invensun interface: ") += e.what()).c_str(), QMessageBox::Ok);
            if(bStart)
                _7i_stop();
            if(bConnect)
                _7i_device_disconnect();
            return;
        }

        ui->linkButton->setText("Stop Device");
    }
}

void MainWindow::sendLSLData(const _7I_EYE_DATA_EX* eyesData)
{
    if (!m_outlet)
    {
        return;
    }

    float leftConfidence = ((eyesData->left_pupil.pupil_bit_mask >> 0) & 1) && ((eyesData->left_pupil.pupil_bit_mask >> 2) & 1);
    float rightConfidence = ((eyesData->right_pupil.pupil_bit_mask >> 0) & 1) && ((eyesData->right_pupil.pupil_bit_mask >> 2) & 1);

    float sample[14] = {
        eyesData->left_gaze.gaze_point.x,
        eyesData->left_gaze.gaze_point.y,
        eyesData->left_pupil.pupil_center.x,
        eyesData->left_pupil.pupil_center.y,
        eyesData->left_pupil.pupil_diameter,
        eyesData->left_pupil.pupil_distance,
        leftConfidence,

        eyesData->right_gaze.gaze_point.x,
        eyesData->right_gaze.gaze_point.y,
        eyesData->right_pupil.pupil_center.x,
        eyesData->right_pupil.pupil_center.y,
        eyesData->right_pupil.pupil_diameter,
        eyesData->right_pupil.pupil_distance,
        rightConfidence,
    };

    m_outlet->push_sample(sample, lsl::local_clock());
}

void MainWindow::onEyesGazeCallback(const _7I_EYE_DATA_EX* eyesData, void* data)
{
    MainWindow* self = reinterpret_cast<MainWindow*>(data);
    self->sendLSLData(eyesData);
}

