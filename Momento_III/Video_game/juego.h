#ifndef JUEGO_H
#define JUEGO_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QTimer>
#include <QPushButton>
#include <QFontDatabase>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QGraphicsEffect>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsColorizeEffect>
#include <QGraphicsProxyWidget>
#include <QTimeLine>
#include <QGraphicsItemAnimation>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QUrl>

#include "goku.h"
#include "enemigo.h"

/**
 * @brief La clase Juego representa el primer nivel del videojuego estilo plataformas.
 *
 * Controla el entorno del juego, personajes, enemigos, colisiones, scroll, energía,
 * generación de obstáculos y efectos visuales.
 */
class Juego : public QGraphicsView {
    Q_OBJECT

public:
    /** @brief Constructor principal de Juego. */
    explicit Juego(QWidget *parent = nullptr);

    /** @brief Destructor. Libera memoria y recursos. */
    ~Juego();

    /** @brief Inicia el juego. */
    void iniciar();

    /** @brief Aumenta el contador de soldados eliminados. */
    void aumentarContadorSoldados();

    /** @brief Pausa el juego completamente. */
    void pausarJuego();

    /** @brief Reanuda todos los elementos pausados. */
    void reanudarTodo();

    /** @brief Detiene todo el movimiento y animaciones. */
    void detenerTodo();

    /** @brief Alterna entre pausa y reanudación. */
    void togglePausa();

    /** @brief Verifica si el juego está en pausa. */
    bool estaPausado() const { return pausado; }

    /** @brief Añade un item a la escena. */
    void agregarItemEscena(QGraphicsItem* item);

    /** @brief Elimina un item de la escena. */
    void removerItemEscena(QGraphicsItem* item);

    /** @brief Muestra la pantalla de Game Over. */
    void mostrarGameOver();

    /** @brief Reinicia el juego desde cero. */
    void reiniciarJuego();

signals:
    void salirAlMenu();         ///< Señal que notifica el regreso al menú
    void gameOver();            ///< Señal de derrota
    void nivelCompletado();     ///< Señal de victoria al completar el nivel

private slots:
    /** @brief Actualiza elementos del juego (loop). */
    void actualizar();

    /** @brief Genera enemigos en el mapa. */
    void generarEnemigo();

    /** @brief Genera plataformas. */
    void generarPlataforma();

    /** @brief Muestra el menú cuando hay Game Over. */
    void mostrarMenuGameOver();

private:
    // --- Elementos visuales ---
    QGraphicsScene* escena;                      ///< Escena principal del juego
    QGraphicsRectItem* fondoBarra;              ///< Fondo de la barra de energía
    QGraphicsRectItem* barraEnergia;            ///< Barra de energía visible
    QGraphicsTextItem* contadorSoldados;        ///< Texto que muestra soldados eliminados

    // Botones de pausa
    QPushButton* botonPausa;
    QPushButton* btnContinuar;
    QPushButton* btnSalir;
    QGraphicsProxyWidget* proxyContinuar;
    QGraphicsProxyWidget* proxySalir;
    void configurarBotonesPausa();

    // --- Entidades del juego ---
    Goku* goku;                                  ///< Personaje principal
    QVector<Enemigo*> enemigos;                  ///< Lista de enemigos
    QVector<QGraphicsPixmapItem*> capsulas;      ///< Energía o ítems
    QVector<QGraphicsPixmapItem*> plataformas;   ///< Superficies donde se puede pisar
    QVector<QGraphicsPixmapItem*> obstaculos;    ///< Obstáculos peligrosos
    QVector<QGraphicsPixmapItem*> fondosScroll;  ///< Fondos que se mueven

    // Recursos precargados
    QVector<QPixmap> imagenesPlataformas;
    QVector<QPixmap> imagenesTroncos;
    QVector<QPixmap> imagenesRocas;
    QVector<QPixmap> imagenesObstaculos;
    QString dragonBallFont;                      ///< Fuente personalizada

    // Temporizadores
    QTimer* timerJuego;
    QTimer* timerEnemigos;
    QTimer* timerCapsulas;
    QTimer* timerPlataformas;
    QTimer* timerObstaculos;
    QTimer* timerAnimacionCuerda;
    QTimer* timerGeneracionCuerdas;
    QTimer* timerTroncos;
    QTimer* timerGameOver;

    // Estado del juego
    float velocidadScroll = 3.0;                 ///< Velocidad del fondo
    bool pausado = false;                        ///< Pausa activa
    bool pPresionado = false;                    ///< Flag para tecla P
    bool gokuEnCuerda = false;                   ///< Si Goku está colgado
    int soldadosEliminados = 0;                  ///< Contador de soldados
    const int OBJETIVO_SOLDADOS = 1;             ///< Soldados a eliminar para ganar

    // --- Cuerdas ---
    struct Cuerda {
        QPointF origen;                          ///< Punto de origen
        double largo = 215;                      ///< Largo de la cuerda
        double angulo;                           ///< Ángulo de oscilación
        double velocidadAngular = 0;             ///< Velocidad de oscilación
        bool activa = false;                     ///< Si está activa
        QGraphicsPathItem* cuerdaItem = nullptr; ///< Representación gráfica
        QGraphicsPixmapItem* gokuSprite = nullptr; ///< Sprite de Goku colgado
        bool gokuAgarrado = false;               ///< Si Goku está colgado
        QPointF puntoMedio;                      ///< Centro visual
    };
    QVector<Cuerda> cuerdas;

    // --- Troncos giratorios ---
    struct TroncoGiratorio {
        QGraphicsPixmapItem* sprite;
        qreal velocidadY;
        qreal velocidadX;
        qreal velocidadRotacion;
        qreal rotacionActual;
        bool enSuelo;
    };
    TroncoGiratorio troncoActual;

    // --- Lógica interna ---
    void actualizarBarraEnergia();
    void generarObstaculo();
    void generarCuerda();
    void actualizarCuerda();
    void activarCuerda(Goku* goku);
    void soltarGokuDeCuerda(Cuerda& cuerda);
    QPointF calcularExtremo(const Cuerda& cuerda) const;
    void generarTroncoUnico();
    void actualizarTronco();

    // --- Detección de colisión entre tronco y plataformas ---
    QGraphicsPixmapItem* detectarPlataformaSobre(TroncoGiratorio& tronco) {
        QRectF areaTronco = tronco.sprite->boundingRect().translated(tronco.sprite->pos());
        for (auto plataforma : plataformas) {
            QRectF areaPlataforma = plataforma->boundingRect().translated(plataforma->pos());
            if (areaTronco.bottom() >= areaPlataforma.top() - 5 &&
                areaTronco.bottom() <= areaPlataforma.top() + 15 &&
                areaTronco.right() > areaPlataforma.left() &&
                areaTronco.left() < areaPlataforma.right()) {
                return plataforma;
            }
        }
        return nullptr;
    }

    // --- Decoración visual ---
    QVector<QPixmap> spritesPalmeras;                   ///< Sprites de palmeras
    QVector<QGraphicsPixmapItem*> decoracionPalmeras;  ///< Palmeras activas
    QGraphicsPixmapItem* pajaroItem;                   ///< Pájaro animado
    int frameActualPajaro;                             ///< Frame actual del pájaro
    QVector<QPixmap> framesPajaro;                     ///< Frames del pájaro
    QTimer* timerAnimacionPajaro;                      ///< Timer de animación del pájaro

    void cargarAssetsDecoracion();
    void generarDecoracionPalmeras();
    void actualizarDecoracion();
    void iniciarAnimacionPajaro();
    void animarPajaro();

    // --- Sonido ---
    QMediaPlayer* musicaNivel1 = nullptr;              ///< Reproductor de música
    QAudioOutput* audioNivel1 = nullptr;               ///< Salida de audio

protected:
    /** @brief Maneja eventos de teclas presionadas. */
    void keyPressEvent(QKeyEvent* event) override;

    /** @brief Maneja eventos de teclas liberadas. */
    void keyReleaseEvent(QKeyEvent* event) override;
};

#endif // JUEGO_H
