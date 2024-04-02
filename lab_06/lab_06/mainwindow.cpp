#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <vector>
#include <map>
#include <string>
#include <cmath>


#include "qcustomplot.h"

vector<map<string, float>> project_mod_consts = {
        {{"c1", 3.2}, {"p1", 1.05}, {"c2", 2.5}, {"p2", 0.38}},
        {{"c1", 3},   {"p1", 1.12}, {"c2", 2.5}, {"p2", 0.35}},
        {{"c1", 2.8}, {"p1", 1.2},  {"c2", 2.5}, {"p2", 0.32}},
};

vector<float> pm_percents = {8, 18, 25, 26, 31, 108};
vector<float> tm_percents = {36, 36, 18, 18, 28, 136};
vector<float> wbs_percents = {4, 12, 44, 6, 14, 7, 7, 6, 100};

float calc_PM(float c1, float EAF, float size, float p1) {
    return c1 * EAF * pow(size, p1);
}

float calc_TM(float c2, float PM, float p2) {
    return c2 * pow(PM, p2);
}

MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    connect(ui->calcPB, SIGNAL(clicked()), this, SLOT(calc()));
    connect(ui->modePlotPB, SIGNAL(clicked()), this, SLOT(mode_plot()));
    connect(ui->eafPlotPB, SIGNAL(clicked()), this, SLOT(eaf_plot()));
    connect(ui->workerPB, SIGNAL(clicked()), this, SLOT(calk_workers()));

    eaf_coeff = {
            {ui->relyComboBox, {0.75, 0.86, 1.0,  1.15, 1.40}},
            {ui->dataComboBox, {0.94, 1.0,  1.08, 1.16}},
            {ui->cplxComboBox, {0.70, 0.85, 1.0,  1.15, 1.30}},
            {ui->timeComboBox, {1.00, 1.11, 1.50}},
            {ui->storComboBox, {1.00, 1.06, 1.21}},
            {ui->virtComboBox, {0.87, 1.00, 1.15, 1.30}},
            {ui->turnComboBox, {0.87, 1.00, 1.07, 1.15}},
            {ui->acapComboBox, {1.46, 1.19, 1.00, 0.86, 0.71}},
            {ui->aexpComboBox, {1.29, 1.15, 1.00, 0.91, 0.82}},
            {ui->pcapComboBox, {1.42, 1.17, 1.00, 0.86, 0.70}},
            {ui->vexpComboBox, {1.21, 1.10, 1.00, 0.90}},
            {ui->lexpComboBox, {1.14, 1.07, 1.00, 0.95}},
            {ui->modpComboBox, {1.24, 1.10, 1.00, 0.91, 0.82}},
            {ui->toolComboBox, {1.24, 1.10, 1.00, 0.91, 0.82}},
            {ui->scedComboBox, {1.23, 1.08, 1.00, 1.04, 1.10}},
    };
}

MainWindow::~MainWindow() {
    delete ui;
}

float MainWindow::calc_EAF_ui() {
    float res = 1;

    for (const auto &[combo_box, arr]: eaf_coeff) {
        int index = combo_box->currentIndex();

        res *= arr[index];
    }

    return res;
}

float MainWindow::calc_PM_ui(float EAF) {
    int project_mode = ui->projectModeComboBox->currentIndex();
    float c1 = project_mod_consts[project_mode]["c1"],
            p1 = project_mod_consts[project_mode]["p1"];
    float size = ui->sizeSpinBox->value();

//    return c1 * EAF * pow(size, p1);
    return calc_PM(c1, EAF, size, p1);
}

float MainWindow::calc_TM_ui(float PM) {
    int project_mode = ui->projectModeComboBox->currentIndex();
    float c2 = project_mod_consts[project_mode]["c2"],
            p2 = project_mod_consts[project_mode]["p2"];

//    return c2 * pow(PM, p2);
    return calc_TM(c2, PM, p2);
}

void MainWindow::calc() {
    float eaf = calc_EAF_ui();
    float pm = calc_PM_ui(eaf), all_pm = pm * (1 + pm_percents[0] / 100.0);
    float tm = calc_TM_ui(pm), all_tm = tm * (1 + tm_percents[0] / 100.0);

    ui->lineEditPM->setText(QString::number(all_pm, 'f', 2));
    ui->lineEditTM->setText(QString::number(all_tm, 'f', 2));

    for (int i = 0; i < pm_percents.size(); i++) {
        float cur_pm = pm * (pm_percents[i] / 100.0);
        float cur_tm = tm * (tm_percents[i] / 100.0);

        ui->tableLifeStages->setItem(i, 0, new QTableWidgetItem(QString::number(cur_pm, 'f', 2)));
        ui->tableLifeStages->setItem(i, 1, new QTableWidgetItem(QString::number(cur_tm, 'f', 2)));
    }

    for (int i = 0; i < wbs_percents.size(); i++) {
        float p_m = all_pm * (wbs_percents[i] / 100.0);

        ui->tableWBS->setItem(i, 1, new QTableWidgetItem(QString::number(p_m, 'f', 2)));
    }
}

QCustomPlot *get_mode_plot(const QVector<double> &x, const QVector<double> &y) {
    auto customPlot = new QCustomPlot();
    customPlot->setLocale(QLocale(QLocale::Russian));

    QSharedPointer<QCPAxisTickerText> textTicker(new QCPAxisTickerText);
    customPlot->xAxis->setTicker(textTicker);
    textTicker->addTick(1.0, "обычный");
    textTicker->addTick(2.0, "промежуточный");
    textTicker->addTick(3.0, "встроенный");

    customPlot->setMinimumSize(450, 250);
    auto graph = customPlot->addGraph();
    graph->setData(x, y);
    customPlot->yAxis->rescale();
    customPlot->xAxis->setRange(0, 4);
    customPlot->replot();

    return customPlot;
}

void MainWindow::mode_plot() {
    auto dialog = new QDialog(this);
    auto layout = new QHBoxLayout(dialog);

    QVector<double> x{1, 2, 3}, pm, tm;
    float eaf = 1, size = 100;
    for (auto &it: project_mod_consts) {
        double cur_pm = calc_PM(it["c1"], eaf, size, it["p1"]), all_pm = cur_pm * (1 + pm_percents[0] / 100.0);
        double cur_tm = calc_TM(it["c2"], cur_pm, it["p2"]), all_tm = cur_tm * (1 + tm_percents[0] / 100.0);

        pm.append(all_pm);
        tm.append(all_tm);
    }

    auto left_layout = new QVBoxLayout(dialog);
    auto pm_label = new QLabel("Трудозатраты");
    auto pm_plot = get_mode_plot(x, pm);
    left_layout->addWidget(pm_label);
    left_layout->addWidget(pm_plot);
    left_layout->addStretch();

    auto right_layout = new QVBoxLayout(dialog);
    auto tm_label = new QLabel("Время");
    auto tm_plot = get_mode_plot(x, tm);
    right_layout->addWidget(tm_label);
    right_layout->addWidget(tm_plot);
    right_layout->addStretch();

    layout->addLayout(left_layout);
    layout->addLayout(right_layout);
    dialog->setLayout(layout);

    dialog->resize(1000, 300);
    dialog->show();
}

pair<QVector<QVector<double>>, QVector<QVector<double>>> get_eaf_data(QVector<double> x_val) {
    QVector<QVector<double>> pm(3), tm(3);
    float size = 100;
    for (int i = 0; i < x_val.size(); ++i) {
        float eaf = x_val[i];

        for (int j = 0; j < project_mod_consts.size(); ++j) {
            auto it = project_mod_consts[j];
            double cur_pm = calc_PM(it["c1"], eaf, size, it["p1"]), all_pm = cur_pm * (1 + pm_percents[0] / 100.0);
            double cur_tm = calc_TM(it["c2"], cur_pm, it["p2"]), all_tm = cur_tm * (1 + tm_percents[0] / 100.0);

            pm[j].append(all_pm);
            tm[j].append(all_tm);
        }
    }

    return {pm, tm};
}

QCustomPlot *get_eaf_plot(QVector<double> x_val, QVector<QVector<double>> data) {
    QVector<double> x_labels{0, 2, 4, 6, 8};
    QVector<QString> labels = {"Очень низкий", "Низкий", "Номинальный", "Высокий", "Очень высокий"};
    QVector<QPen> colors = {QPen(Qt::blue), QPen(Qt::red), QPen(Qt::green)};
    QVector<QString> names = {"обычный", "промежуточный", "встроенный"};

    auto customPlot = new QCustomPlot();
    customPlot->setLocale(QLocale(QLocale::Russian));

    QSharedPointer<QCPAxisTickerText> textTicker(new QCPAxisTickerText);
    customPlot->xAxis->setTicker(textTicker);
    for (int i = 0; i < x_val.size(); ++i) {
        textTicker->addTick(x_labels[i], labels[i]);
    }

    for (int i = 0; i < colors.size(); i++) {
        auto graph = customPlot->addGraph();
        graph->setData(x_labels, data[i]);
        graph->setPen(colors[i]);
        graph->setName(names[i]);
    }

    customPlot->setMinimumSize(450, 300);
    customPlot->legend->setVisible(true);
    customPlot->yAxis->rescale();
    customPlot->xAxis->setRange(x_labels[0] - 1, x_labels[4] + 1);
    customPlot->replot();

    return customPlot;
}

void MainWindow::eaf_plot() {
    auto dialog = new QDialog(this);
    auto layout = new QHBoxLayout(dialog);


    auto left_layout = new QVBoxLayout(dialog);

    auto acap_label = new QLabel("ACAP - способности аналитика");
    left_layout->addWidget(acap_label);
    auto acap_layout = new QHBoxLayout(dialog);
    QVector<double> acap_x_val = {1.46, 1.19, 1, 0.86, 0.71};
    auto acap_data = get_eaf_data(acap_x_val);
    auto acap_plot_pm = get_eaf_plot(acap_x_val, acap_data.first);
    auto acap_plot_tm = get_eaf_plot(acap_x_val, acap_data.second);
    acap_layout->addWidget(acap_plot_pm);
    acap_layout->addWidget(acap_plot_tm);
    left_layout->addLayout(acap_layout);


    auto pcap_label = new QLabel("PCAP - способности программиста");
    left_layout->addWidget(pcap_label);
    auto pcap_layout = new QHBoxLayout(dialog);
    QVector<double> pcap_x_val = {1.42, 1.17, 1, 0.86, 0.7};
    auto pcap_data = get_eaf_data(pcap_x_val);
    auto pcap_plot_pm = get_eaf_plot(pcap_x_val, pcap_data.first);
    auto pcap_plot_tm = get_eaf_plot(pcap_x_val, pcap_data.second);
    pcap_layout->addWidget(pcap_plot_pm);
    pcap_layout->addWidget(pcap_plot_tm);
    left_layout->addLayout(pcap_layout);

    left_layout->addStretch();

    auto right_layout = new QVBoxLayout(dialog);

    auto modp_label = new QLabel("MODP - использование современных методов");
    right_layout->addWidget(modp_label);
    auto modp_layout = new QHBoxLayout(dialog);
    QVector<double> modp_x_val = {1.24, 1.1, 1, 0.91, 0.82};
    auto modp_data = get_eaf_data(modp_x_val);
    auto modp_plot_pm = get_eaf_plot(modp_x_val, modp_data.first);
    auto modp_plot_tm = get_eaf_plot(modp_x_val, modp_data.second);
    modp_layout->addWidget(modp_plot_pm);
    modp_layout->addWidget(modp_plot_tm);
    right_layout->addLayout(modp_layout);

    auto tool_label = new QLabel("TOOL - использование программных инструментов");
    right_layout->addWidget(tool_label);
    auto tool_layout = new QHBoxLayout(dialog);
    QVector<double> tool_x_val = {1.24, 1.1, 1, 0.91, 0.82};
    auto tool_data = get_eaf_data(tool_x_val);
    auto tool_plot_pm = get_eaf_plot(tool_x_val, tool_data.first);
    auto tool_plot_tm = get_eaf_plot(tool_x_val, tool_data.second);
    tool_layout->addWidget(tool_plot_pm);
    tool_layout->addWidget(tool_plot_tm);
    right_layout->addLayout(tool_layout);

    right_layout->addStretch();

    layout->addLayout(left_layout);
    layout->addLayout(right_layout);
    dialog->setLayout(layout);
    dialog->resize(1600, 700);
    dialog->show();
}

void MainWindow::calk_workers() {
    float eaf = calc_EAF_ui();
    float pm = calc_PM_ui(eaf), all_pm = pm * (1 + pm_percents[0] / 100.0);
    float tm = calc_TM_ui(pm), all_tm = tm * (1 + tm_percents[0] / 100.0);

    int sz1 = pm_percents.size() - 1;
    QVector<double> people_data(sz1), tm_data(sz1);
    for (int i = 0; i < sz1; i++) {
        float cur_pm = pm * (pm_percents[i] / 100.0);
        float cur_tm = tm * (tm_percents[i] / 100.0);

        people_data[i] = ceil(cur_pm / cur_tm);
        tm_data[i] = cur_tm;
    }

    auto customPlot = new QCustomPlot();
    auto *regen = new QCPBars(customPlot->xAxis, customPlot->yAxis);
    regen->setAntialiased(false);

    double last_time = tm_data[0];
    int index = 0;
    for (int i = 1; i <= int(ceil(all_tm)); i++) {
        if(last_time < 1){
            if(last_time < 0.5){
                if (index < people_data.size() - 1) index++;
                regen->addData(i, people_data[index]);
            }else{
                regen->addData(i, people_data[index]);
                if (index < people_data.size() - 1) index++;
            }
            last_time += tm_data[index];
        } else{
            regen->addData(i, people_data[index]);
        }

        last_time--;
    }

    customPlot->yAxis->rescale();
    customPlot->xAxis->setRange(0, ceil(all_tm) + 1);
    customPlot->yAxis->setLabel("Количество сотрудников");
    customPlot->xAxis->setLabel("Неделя");
    customPlot->replot();

    auto dialog = new QDialog(this);
    auto layout = new QHBoxLayout(dialog);
    layout->addWidget(customPlot);
    dialog->setLayout(layout);
    dialog->resize(1600, 700);
    dialog->show();
}
