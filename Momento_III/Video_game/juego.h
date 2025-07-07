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

#include "goku.h"
#include "enemigo.h"

class Juego : public QGraphicsView {
    Q_OBJECT

public:
    explicit Juego(QWidget *parent = nullptr);
    ~Juego();

    void iniciar();
    void aumentarContadorSoldados();
    void pausarJuego();
    void reanudarTodo();
    void detenerTodo();
    void togglePausa();
    bool estaPausado() const { return pausado; }

    void agregarItemEscena(QGraphicsItem* item);
    void removerItemEscena(QGraphicsItem* item);
    void mostrarGameOver();
    void reiniciarJuego();

signals:
    void salirAlMenu();   // Vuelve a MainWindow
    void gameOver();      // Señal de finalización
    void nivelCompletado();  // 🔔 Nivel 1 finalizado

private slots:
    void actualizar();             // Ciclo principal
    void generarEnemigo();
    void generarPlataforma();
    void mostrarMenuGameOver();

private:
    // Escena y UI
    QGraphicsScene* escena;
    QGraphicsRectItem* fondoBarra;
    QGraphicsRectItem* barraEnergia;
    QGraphicsTextItem* contadorSoldados;

    // Botones de pausa
    QPushButton* botonPausa;
    QPushButton* btnContinuar;
    QPushButton* btnSalir;
    QGraphicsProxyWidget* proxyContinuar;
    QGraphicsProxyWidget* proxySalir;
    void configurarBotonesPausa();

    // Personajes y elementos del juego
    Goku* goku;
    QVector<Enemigo*> enemigos;
    QVector<QGraphicsPixmapItem*> capsulas;
    QVector<QGraphicsPixmapItem*> plataformas;
    QVector<QGraphicsPixmapItem*> obstaculos;
    QVector<QGraphicsPixmapItem*> fondosScroll;

    // Recursos gráficos cargados una vez
    QVector<QPixmap> imagenesPlataformas;
    QVector<QPixmap> imagenesTroncos;
    QVector<QPixmap> imagenesRocas;
    QVector<QPixmap> imagenesObstaculos;
    QString dragonBallFont;

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

    // Variables de control
    float velocidadScroll = 3.0;
    bool pausado = false;
    bool pPresionado = false;
    bool gokuEnCuerda = false;
    int soldadosEliminados = 0;
    const int OBJETIVO_SOLDADOS = 3;

    // Estructura de la cuerda
    struct Cuerda {
        QPointF origen;
        double largo = 215;
        double angulo;
        double velocidadAngular = 0;
        bool activa = false;
        QGraphicsPathItem* cuerdaItem = nullptr;
        QGraphicsPixmapItem* gokuSprite = nullptr;
        bool gokuAgarrado = false;
        QPointF puntoMedio;
    };
    QVector<Cuerda> cuerdas;

    // Estructura del tronco giratorio
    struct TroncoGiratorio {
        QGraphicsPixmapItem* sprite;
        qreal velocidadY;
        qreal velocidadX;
        qreal velocidadRotacion;
        qreal rotacionActual;
        bool enSuelo;
    };
    TroncoGiratorio troncoActual;

    // Métodos de juego
    void moverFondo();
    void actualizarBarraEnergia();
    void generarObstaculo();
    void generarCuerda();
    void actualizarCuerda();
    void activarCuerda(Goku* goku);
    void soltarGokuDeCuerda(Cuerda& cuerda);
    QPointF calcularExtremo(const Cuerda& cuerda) const;
    void generarTroncoUnico();
    void actualizarTronco();

    // Detección de colisión tronco-plataforma
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

    // Decoración visual
    QVector<QPixmap> spritesPalmeras;
    QVector<QGraphicsPixmapItem*> decoracionPalmeras;
    QGraphicsPixmapItem* pajaroItem;
    int frameActualPajaro;
    QVector<QPixmap> framesPajaro;
    QTimer* timerAnimacionPajaro;

    void cargarAssetsDecoracion();
    void generarDecoracionPalmeras();
    void actualizarDecoracion();
    void iniciarAnimacionPajaro();
    void animarPajaro();

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
};

#endif // JUEGO_H
