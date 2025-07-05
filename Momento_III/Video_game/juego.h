#ifndef JUEGO_H
#define JUEGO_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QTimer>
#include <QPushButton>
#include <QFontDatabase>          // Para manejo de fuentes
#include <QPropertyAnimation>     // Para animaciones
#include <QEasingCurve>           // Para curvas de animación
#include <QGraphicsEffect>        // Para efectos visuales
#include <QGraphicsDropShadowEffect> // Para sombras
#include <QGraphicsColorizeEffect>   // Para efecto de color
#include <QTimeLine>
#include <QGraphicsItemAnimation>
#include "goku.h"
#include "enemigo.h"

class Juego : public QGraphicsView {
    Q_OBJECT
public:
    Juego(QWidget *parent = nullptr);
    void iniciar();
    void aumentarContadorSoldados();
    void pausarJuego();
    void agregarItemEscena(QGraphicsItem* item);
    void removerItemEscena(QGraphicsItem* item);
    void togglePausa();
    bool estaPausado() const { return pausado; }
    void detenerTodo();
    void reanudarTodo();

private slots:
    void actualizar();
    void generarEnemigo();
    void generarPlataforma();

private:
    // Elementos gráficos y escena
    QGraphicsScene* escena;
    QGraphicsRectItem* fondoBarra;
    QGraphicsRectItem* barraEnergia;
    QGraphicsTextItem* contadorSoldados;
    QPushButton* botonPausa;
    QString dragonBallFont;

    // Personajes y objetos del juego
    Goku* goku;
    QVector<Enemigo*> enemigos;
    QVector<QGraphicsPixmapItem*> capsulas;
    QVector<QGraphicsPixmapItem*> plataformas;
    QVector<QGraphicsPixmapItem*> obstaculos;
    QVector<QPixmap> imagenesPlataformas;
    QVector<QPixmap> imagenesTroncos;
    QVector<QPixmap> imagenesRocas;
    QVector<QPixmap> imagenesObstaculos;
    QVector<QGraphicsPixmapItem*> fondosScroll;

    // Timers del juego
    QTimer* timerJuego;
    QTimer* timerEnemigos;
    QTimer* timerCapsulas;
    QTimer* timerPlataformas;
    QTimer* timerObstaculos;
    QTimer* timerAnimacionCuerda;
    QTimer* timerGeneracionCuerdas;
    QTimer* timerTroncos;

    // Variables de estado
    float velocidadScroll = 3.0;
    bool gokuEnCuerda = false;
    bool pausado = false;
    bool pPresionado = false;
    int soldadosEliminados = 0;
    const int OBJETIVO_SOLDADOS = 5;

    // Estructuras especiales
    struct Cuerda {
        QPointF origen;
        double largo = 220;
        double angulo;
        double velocidadAngular = 0;
        bool activa = false;
        QGraphicsPathItem* cuerdaItem = nullptr;
        QGraphicsPixmapItem* gokuSprite = nullptr;
        bool gokuAgarrado = false;
        QPointF puntoMedio;
    };
    QVector<Cuerda> cuerdas;

    struct TroncoGiratorio {
        QGraphicsPixmapItem* sprite;
        qreal velocidadY;
        qreal velocidadX;
        qreal velocidadRotacion;
        qreal rotacionActual;
        bool enSuelo;
    };
    TroncoGiratorio troncoActual;

    // Métodos privados
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

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
};

#endif // JUEGO_H
