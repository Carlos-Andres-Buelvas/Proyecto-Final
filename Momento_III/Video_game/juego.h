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
        double largo = 275;
        double angulo; // Ángulo actual en radianes
        double velocidadAngular = 0;
        bool activa = false;
        QGraphicsLineItem* linea = nullptr;
        QGraphicsPixmapItem* gokuSprite = nullptr;
        bool gokuAgarrado = false; // Nuevo: indica si Goku está agarrado a esta cuerda
        //int tiempoEnCuerda = 0; // Contador de frames en la cuerda
        //const int maxTiempoEnCuerda = 180; // 3 segundos (60fps * 3)
    };

    QVector<Cuerda> cuerdas;  // múltiples cuerdas
    QTimer* timerAnimacionCuerda = nullptr;  // Para actualizar el péndulo (60 FPS)
    QTimer* timerGeneracionCuerdas = nullptr; // Para generar nuevas cuerdas cada X segundos
    bool gokuEnCuerda = false;
    void generarCuerda();     // para crear nuevas cuerdas
    void actualizarCuerda();  // para actualizar el péndulo
    void activarCuerda(Goku* goku);
    void soltarGokuDeCuerda(Cuerda& cuerda);

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;

};

#endif // JUEGO_H
