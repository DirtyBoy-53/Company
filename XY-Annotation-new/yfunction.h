#pragma once
#include <QFiledialog>


inline QString chooseFile()
{
    return QFileDialog::getOpenFileName(nullptr, "Open File", "./",
        "Images (*.png *.bmp *.jpg *.json)");
}