#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "juego.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    juego = nullptr;
}

MainWindow::~MainWindow()
{
    delete ui;
    delete juego;
}

void MainWindow::on_newGameButton_clicked()
{
    if (juego) {
        delete juego;
    }

    juego = new Juego(); // SIN el "this" como parent
    juego->show();

    this->hide();
}

