#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFontDatabase>  // Para manejo de fuentes
#include <QMessageBox>
#include "juego.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_newGameButton_clicked();

private:
    Ui::MainWindow *ui;
    Juego* juego;
    QString dragonBallFont;  // Para almacenar el nombre de la fuente

    // Método para cargar la fuente
    QString loadDragonBallFont();
};

#endif // MAINWINDOW_H
