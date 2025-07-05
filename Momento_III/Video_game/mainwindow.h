#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFontDatabase>  // Para manejo de fuentes personalizadas
#include <QMessageBox>    // Para mostrar alertas o mensajes
#include "juego.h"        // Incluye la clase principal del juego

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);  // Constructor
    ~MainWindow();                                    // Destructor

private slots:
    void on_newGameButton_clicked();  // Slot para iniciar el juego

private:
    Ui::MainWindow *ui;
    Juego* juego;                     // Referencia al objeto del juego
    QString dragonBallFont;          // Nombre de la fuente personalizada

    QString loadDragonBallFont();    // Carga y retorna la fuente Dragon Ball
};

#endif // MAINWINDOW_H
