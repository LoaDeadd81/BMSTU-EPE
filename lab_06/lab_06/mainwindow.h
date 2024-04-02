#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QComboBox>
#include <vector>

using namespace std;

QT_BEGIN_NAMESPACE
namespace Ui {
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

    ~MainWindow();

private:
    Ui::MainWindow *ui;

    float calc_EAF_ui();
    float calc_PM_ui(float EAF);
    float calc_TM_ui(float PM);

    map<QComboBox *, vector<float>> eaf_coeff;

private slots:
    void calc();
    void mode_plot();
    void eaf_plot();
    void calk_workers();
};

#endif // MAINWINDOW_H
