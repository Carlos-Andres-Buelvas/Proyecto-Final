#ifndef ENEMIGO_H
#define ENEMIGO_H

#include "personaje.h"
#include <QVector>
#include <QPixmap>
#include <QTimer>
#include <QGraphicsScene>

class Goku;

class Enemigo : public Personaje {
    Q_OBJECT
public:
    Enemigo(float x, float y, float ancho, float alto, Goku* gokuRef);

    void mover() override;
    void disparar(QGraphicsScene* escena) override;
    bool estaDisparando() const { return estado == Disparando; }
    void eliminarProyectiles();
    void detenerProyectiles();
    void reanudarProyectiles();
    void setPausado(bool pausa);

    QVector<QPair<QGraphicsEllipseItem*, QTimer*>> proyectilesActivos;

private:
    void cargarAnimaciones();
    void iniciarDisparo();
    void crearDisparo();

    enum EstadoEnemigo { Patrullando, Disparando, Muerto };
    EstadoEnemigo estado = Patrullando;

    Goku* gokuDetectado = nullptr;
    QVector<QPixmap> framesIzquierda;
    QVector<QPixmap> framesDerecha;

    int direccion;
    int frameActual;
    int contador;
    int velocidadAnimacion;
    float velocidadMovimiento;
    bool pausado = false;

    bool disparando = false;
    QTimer* timerDisparo;
};

#endif // ENEMIGO_H
