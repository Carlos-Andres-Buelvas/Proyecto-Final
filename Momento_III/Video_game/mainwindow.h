#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFontDatabase>  ///< Para manejo de fuentes personalizadas
#include <QMessageBox>    ///< Para mostrar alertas o mensajes
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QUrl>
#include "juego.h"
#include "juego2.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

/**
 * @brief Ventana principal del videojuego.
 *
 * Administra el menú inicial, carga de niveles, fuentes, música y navegación entre vistas.
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    /**
     * @brief Constructor de MainWindow.
     * @param parent Widget padre (por defecto nullptr).
     */
    explicit MainWindow(QWidget *parent = nullptr);

    /**
     * @brief Inicia el segundo nivel del juego.
     */
    void iniciarNivel2();

    /**
     * @brief Destructor de MainWindow.
     */
    ~MainWindow();

private slots:
    /**
     * @brief Slot llamado al presionar "Nuevo Juego".
     */
    void on_newGameButton_clicked();

    /**
     * @brief Slot llamado al presionar "Información".
     */
    void on_informationButton_clicked();

private:
    Ui::MainWindow *ui;           ///< Interfaz generada por Qt Designer
    Juego* juego = nullptr;      ///< Instancia del nivel 1
    Juego2* juego2 = nullptr;    ///< Instancia del nivel 2
    QString dragonBallFont;      ///< Nombre de la fuente personalizada

    /**
     * @brief Carga la fuente Dragon Ball desde los recursos.
     * @return Nombre de la fuente cargada o "Arial" si falla.
     */
    QString loadDragonBallFont();

    /**
     * @brief Aplica estilos visuales y fuente personalizada al menú.
     */
    void aplicarEstilosMenu();

    /**
     * @brief Devuelve al usuario al menú principal.
     */
    void volverAlMenu();

    /**
     * @brief Muestra un diálogo de Game Over.
     * @param victoria Si es true, muestra victoria; de lo contrario, derrota.
     */
    void mostrarGameOverDialog(bool victoria);

    /**
     * @brief Muestra el título de un nivel antes de iniciarlo.
     * @param titulo Texto a mostrar.
     * @param vistaJuego Vista del nivel a mostrar.
     * @param tieneIniciar Indica si debe llamarse `iniciar()` al finalizar.
     */
    void mostrarTituloNivel(const QString& titulo, QGraphicsView* vistaJuego, bool tieneIniciar = true);

    // --- Sonido ---
    QMediaPlayer* musicaMenu = nullptr;          ///< Reproductor de música del menú
    QAudioOutput* salidaAudioMenu = nullptr;     ///< Salida de audio del menú
};

#endif // MAINWINDOW_H
