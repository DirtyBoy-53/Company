#include "algparam.h"
#include "ui_algparam.h"

AlgParam::AlgParam(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AlgParam)
{
    ui->setupUi(this);
}

AlgParam::~AlgParam()
{
    delete ui;
}

void AlgParam::readParamFromUI(shape_json::root_s &root)
{
    root.camera_in_11_fx = ui->Edit_CameraIn11_fx->text().toDouble();
    root.camera_in_12    = ui->Edit_CameraIn12->text().toDouble()   ;
    root.camera_in_13_Cx = ui->Edit_CameraIn13_cx->text().toDouble();
    root.camera_in_21    = ui->Edit_CameraIn21->text().toDouble()   ;
    root.camera_in_22_fy = ui->Edit_CameraIn22_fy->text().toDouble();
    root.camera_in_23_Cy = ui->Edit_CameraIn23_cy->text().toDouble();
    root.camera_in_31    = ui->Edit_CameraIn31->text().toDouble()   ;
    root.camera_in_32    = ui->Edit_CameraIn32->text().toDouble()   ;
    root.camera_in_33    = ui->Edit_CameraIn33->text().toDouble()   ;

    root.rotate_X = ui->Edit_RotateX->text().toDouble();
    root.rotate_Y = ui->Edit_RotateY->text().toDouble();
    root.rotate_Z = ui->Edit_RotateZ->text().toDouble();

    root.translate_X = ui->Edit_TranslationX->text().toDouble();
    root.translate_Y = ui->Edit_TranslationY->text().toDouble();
    root.translate_Z = ui->Edit_TranslationZ->text().toDouble();

    root.distortion_k1 = ui->Edit_Distortion_k1->text().toDouble();
    root.distortion_k2 = ui->Edit_Distortion_k2->text().toDouble();
    root.distortion_k3 = ui->Edit_Distortion_k3->text().toDouble();
    root.distortion_p1 = ui->Edit_Distortion_p1->text().toDouble();
    root.distortion_p2 = ui->Edit_Distortion_p2->text().toDouble();

    root.radar_in_11 = ui->Edit_RadarIn_h11->text().toDouble();
    root.radar_in_12 = ui->Edit_RadarIn_h12->text().toDouble();
    root.radar_in_13 = ui->Edit_RadarIn_h13->text().toDouble();
    root.radar_in_21 = ui->Edit_RadarIn_h21->text().toDouble();
    root.radar_in_22 = ui->Edit_RadarIn_h22->text().toDouble();
    root.radar_in_23 = ui->Edit_RadarIn_h23->text().toDouble();
    root.radar_in_31 = ui->Edit_RadarIn_h31->text().toDouble();
    root.radar_in_32 = ui->Edit_RadarIn_h32->text().toDouble();
    root.radar_in_33 = ui->Edit_RadarIn_h33->text().toDouble();
}

void AlgParam::writeParamToUI(const shape_json::root_s &root)
{
    ui->Edit_CameraIn11_fx->setText(QString::number(root.camera_in_11_fx));
    ui->Edit_CameraIn12->setText(QString::number(   root.camera_in_12   ));
    ui->Edit_CameraIn13_cx->setText(QString::number(root.camera_in_13_Cx));
    ui->Edit_CameraIn21->setText(QString::number(   root.camera_in_21   ));
    ui->Edit_CameraIn22_fy->setText(QString::number(root.camera_in_22_fy));
    ui->Edit_CameraIn23_cy->setText(QString::number(root.camera_in_23_Cy));
    ui->Edit_CameraIn31->setText(QString::number(   root.camera_in_31   ));
    ui->Edit_CameraIn32->setText(QString::number(   root.camera_in_32   ));
    ui->Edit_CameraIn33->setText(QString::number(   root.camera_in_33   ));

    ui->Edit_RotateX->setText(QString::number(root.rotate_X));
    ui->Edit_RotateY->setText(QString::number(root.rotate_Y));
    ui->Edit_RotateZ->setText(QString::number(root.rotate_Z));

    ui->Edit_TranslationX->setText(QString::number(root.translate_X));
    ui->Edit_TranslationY->setText(QString::number(root.translate_Y));
    ui->Edit_TranslationZ->setText(QString::number(root.translate_Z));

    ui->Edit_Distortion_k1->setText(QString::number(root.distortion_k1));
    ui->Edit_Distortion_k2->setText(QString::number(root.distortion_k2));
    ui->Edit_Distortion_k3->setText(QString::number(root.distortion_k3));
    ui->Edit_Distortion_p1->setText(QString::number(root.distortion_p1));
    ui->Edit_Distortion_p2->setText(QString::number(root.distortion_p2));

    ui->Edit_RadarIn_h11->setText(QString::number(root.radar_in_11));
    ui->Edit_RadarIn_h12->setText(QString::number(root.radar_in_12));
    ui->Edit_RadarIn_h13->setText(QString::number(root.radar_in_13));
    ui->Edit_RadarIn_h21->setText(QString::number(root.radar_in_21));
    ui->Edit_RadarIn_h22->setText(QString::number(root.radar_in_22));
    ui->Edit_RadarIn_h23->setText(QString::number(root.radar_in_23));
    ui->Edit_RadarIn_h31->setText(QString::number(root.radar_in_31));
    ui->Edit_RadarIn_h32->setText(QString::number(root.radar_in_32));
    ui->Edit_RadarIn_h33->setText(QString::number(root.radar_in_33));

}
#include <QApplication>
void AlgParam::on_Btn_ReadParam_clicked()
{
    QString filename = QApplication::applicationDirPath()
            + "/iniConfig/sense_fusion.json";
    bool ret = YJsonManager::readJson(filename.toStdString(), m_root);
    writeParamToUI(m_root);
    if(ret) ui->Label_Msg->setText("read success!");
    else ui->Label_Msg->setText("read fail!");
}

void AlgParam::on_Btn_WriteParam_clicked()
{
    QString filename = QApplication::applicationDirPath()
            + "/iniConfig/sense_fusion.json";
    readParamFromUI(m_root);
    bool ret = YJsonManager::writeJson(filename.toStdString(), m_root);
    if(ret) ui->Label_Msg->setText("write success!");
    else ui->Label_Msg->setText("write fail!");
}
