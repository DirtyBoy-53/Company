#ifndef LSIDEWINDOW_H
#define LSIDEWINDOW_H

#include <QWidget>

namespace Ui {
class LSideWindow;
}

class LSideWindow : public QWidget
{
    Q_OBJECT

public:
    explicit LSideWindow(QWidget *parent = nullptr);
    ~LSideWindow();

private slots:
    void btn_clicked();

private:
    void initUI();
    void initConnect();

    Ui::LSideWindow *ui;

};

#endif // LSIDEWINDOW_H
