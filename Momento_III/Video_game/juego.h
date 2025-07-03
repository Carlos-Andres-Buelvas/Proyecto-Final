#ifndef JUEGO_H
#define JUEGO_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QTimer>
#include "goku.h"
#include "enemigo.h"

class Juego : public QGraphicsView {
    Q_OBJECT
public:
    Juego(QWidget *parent = nullptr);
    void iniciar();

private slots:
    void actualizar();
    void generarEnemigo();
    void generarPlataforma(); // método que genera las plataformas

private:
    QGraphicsScene* escena;
    QTimer* timerJuego;
    QTimer* timerEnemigos;
    Goku* goku;

    void moverFondo();
    QGraphicsRectItem* fondoBarra;
    QGraphicsRectItem* barraEnergia;
    void actualizarBarraEnergia();
    QVector<QGraphicsPixmapItem*> capsulas;
    QTimer* timerCapsulas;
    QVector<QGraphicsPixmapItem*> fondosScroll;
    float velocidadScroll = 3.0;
    QVector<Enemigo*> enemigos;

    QVector<QGraphicsPixmapItem*> plataformas;
    QTimer* timerPlataformas;

    // Lista para los obstáculos
    QVector<QGraphicsPixmapItem*> obstaculos;
    QTimer* timerObstaculos;

    // Método que vamos a implementar
    void generarObstaculo();

    QVector<QPixmap> imagenesPlataformas;
    QVector<QPixmap> imagenesTroncos;
    QVector<QPixmap> imagenesRocas;
    QVector<QPixmap> imagenesObstaculos;

    // CUERDA
    struct Cuerda {
        QPointF origen;
        double largo;
        double angulo;
        double velocidadAngular = 0;
        bool activa = false;
        QGraphicsPathItem* cuerdaItem = nullptr; // Cambiamos de LineItem a PathItem
        QGraphicsPixmapItem* gokuSprite = nullptr;
        bool gokuAgarrado = false;
        QPointF puntoMedio; // Para la curvatura
    };

    QVector<Cuerda> cuerdas;  // múltiples cuerdas
    QTimer* timerAnimacionCuerda = nullptr;  // Para actualizar el péndulo (60 FPS)
    QTimer* timerGeneracionCuerdas = nullptr; // Para generar nuevas cuerdas cada X segundos
    bool gokuEnCuerda = false;
    void generarCuerda();     // para crear nuevas cuerdas
    void actualizarCuerda();  // para actualizar el péndulo
    void activarCuerda(Goku* goku);
    void soltarGokuDeCuerda(Cuerda& cuerda);
    QPointF calcularExtremo(const Cuerda& cuerda) const;

    struct TroncoGiratorio {
        QGraphicsPixmapItem* sprite;
        qreal velocidadY;        // Velocidad vertical de caída (hacia abajo)
        qreal velocidadX;        // Velocidad horizontal (0 inicial, luego negativa)
        qreal velocidadRotacion; // Velocidad de rotación (negativa para giro antihorario)
        qreal rotacionActual;
        bool enSuelo;           // Bandera para controlar el cambio de movimiento
    };

    TroncoGiratorio troncoActual; // Solo un tronco activo
    QTimer* timerTroncos;
    void generarTroncoUnico();   // Renombrado para claridad
    void actualizarTronco();

    QGraphicsPixmapItem* detectarPlataformaSobre(TroncoGiratorio& tronco) {
        QRectF areaTronco = tronco.sprite->boundingRect().translated(tronco.sprite->pos());

        for (auto plataforma : plataformas) {
            QRectF areaPlataforma = plataforma->boundingRect().translated(plataforma->pos());

            // Verifica si el tronco está sobre la plataforma (con margen de 5px)
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
