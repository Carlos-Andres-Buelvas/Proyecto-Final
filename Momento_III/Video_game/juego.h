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


protected:
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;

};

#endif // JUEGO_H
