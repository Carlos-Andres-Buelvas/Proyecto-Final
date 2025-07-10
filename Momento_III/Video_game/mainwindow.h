#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFontDatabase>  // Para manejo de fuentes personalizadas
#include <QMessageBox>    // Para mostrar alertas o mensajes
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QUrl>
#include "juego.h"
#include "juego2.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);  // Constructor
    void iniciarNivel2(); // 🔹 Nuevo
    ~MainWindow();                                    // Destructor

private slots:
    void on_newGameButton_clicked();  // Slot para iniciar el juego
    void on_informationButton_clicked();  // Slot para información del juego

private:
    Ui::MainWindow *ui;
    Juego* juego = nullptr;
    Juego2* juego2 = nullptr;
    QString dragonBallFont;          // Nombre de la fuente personalizada

    QString loadDragonBallFont();    // Carga y retorna la fuente Dragon Ball

    void aplicarEstilosMenu();  // Aplica estilo y fuente a botones
    void volverAlMenu();        // Retorna al menú principal
    void mostrarGameOverDialog(bool);
    void mostrarTituloNivel(const QString& titulo, QGraphicsView* vistaJuego, bool tieneIniciar = true);

    //Sonido
    QMediaPlayer* musicaMenu = nullptr;
    QAudioOutput* salidaAudioMenu = nullptr;
};

#endif // MAINWINDOW_H
