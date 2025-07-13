#ifndef JUEGO2_H
#define JUEGO2_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QTimer>
#include <QSet>
#include <QPushButton>
#include <QGraphicsProxyWidget>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QUrl>
#include "goku.h"
#include "enemigo.h"

/**
 * @brief La clase Juego2 representa el segundo nivel del videojuego tipo laberinto cenital.
 *
 * Administra la escena, personajes, ítems, enemigos y lógica de victoria y derrota.
 */
class Juego2 : public QGraphicsView {
    Q_OBJECT

public:
    /** @brief Constructor del nivel 2. */
    explicit Juego2(QWidget* parent = nullptr);

    /** @brief Carga el mapa desde un archivo de texto. */
    void cargarMapa(const QString& archivo);

    Goku* goku = nullptr; ///< Instancia de Goku para el nivel 2

    /** @brief Actualiza visualmente la barra de energía. */
    void actualizarBarraEnergia();

    /** @brief Elimina un enemigo de la escena y la lista. */
    void eliminarEnemigo(Enemigo* enemigo);

    /** @brief Muestra pantalla de Game Over. */
    void mostrarGameOver();

    /** @brief Muestra menú con botones tras perder. */
    void mostrarMenuGameOver();

    /** @brief Inicia el nivel (llamado tras pantalla de título). */
    void iniciar();

    /** @brief Añade un ítem a la escena. */
    void agregarItemEscena(QGraphicsItem* item);

    /** @brief Elimina un ítem de la escena. */
    void removerItemEscena(QGraphicsItem* item);

    /** @brief Reanuda la ejecución del juego. */
    void reanudarTodo();

    /** @brief Detiene la ejecución del juego. */
    void detenerTodo();

signals:
    void gameOver();            ///< Señal emitida cuando se pierde el nivel
    void salirAlMenu();         ///< Señal para regresar al menú principal
    void nivelCompletado();     ///< Señal de victoria del nivel 2

private slots:
    /** @brief Actualiza la lógica del juego periódicamente. */
    void actualizar();

private:
    QGraphicsScene* escena; ///< Escena principal del nivel 2

    QVector<Enemigo*> enemigos;                  ///< Lista de enemigos en escena
    QVector<QGraphicsPixmapItem*> llaves;        ///< Llaves recolectables
    QVector<QGraphicsPixmapItem*> capsulas;      ///< Cápsulas de energía
    QGraphicsPixmapItem* bulma;                  ///< Sprite de Bulma
    QGraphicsPixmapItem* puerta;                 ///< Sprite de la puerta de salida

    int llavesRecogidas = 0;                     ///< Contador de llaves recogidas
    const int TOTAL_LLAVES = 1;                  ///< Llaves necesarias para ganar

    /** @brief Verifica si se han recogido todas las llaves. */
    void verificarVictoria();

    /** @brief Abre la puerta si se cumple la condición de victoria. */
    void abrirPuerta();

    QGraphicsRectItem* barraEnergia = nullptr;   ///< Indicador visual de energía
    QGraphicsTextItem* textoLlaves = nullptr;    ///< Texto que muestra cantidad de llaves

    /** @brief Actualiza el texto del contador de llaves. */
    void actualizarContadorLlaves();

    bool m_nivelCompletado = false;              ///< Estado de victoria alcanzada
    bool gameOverMostrado = false;               ///< Si ya se mostró Game Over
    bool enPausa = false;                        ///< Indica si el juego está pausado

    QTimer* timerActualizar = nullptr;           ///< Timer de actualización principal
    QTimer* timerGameOver;                       ///< Timer para mostrar Game Over
    bool pausado = false;                        ///< Si el juego está en pausa lógica

    // Botones y sus contenedores
    QPushButton* btnContinuar = nullptr;
    QPushButton* btnSalir = nullptr;
    QGraphicsProxyWidget* proxyContinuar = nullptr;
    QGraphicsProxyWidget* proxySalir = nullptr;

    /** @brief Alterna entre pausa y reanudación. */
    void togglePausa();

    /** @brief Configura los botones para el menú de pausa. */
    void configurarBotonesPausa();

    QString dragonBallFont; ///< Fuente personalizada usada en los textos

    // --- Sonido ---
    QMediaPlayer* musicaNivel2 = nullptr;        ///< Reproductor de música nivel 2
    QAudioOutput* audioNivel2 = nullptr;         ///< Salida de audio del nivel 2
};

#endif // JUEGO2_H
