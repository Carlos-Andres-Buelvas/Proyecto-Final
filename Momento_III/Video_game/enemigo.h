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
    bool estaDisparando() const;
    QVector<QPair<QGraphicsEllipseItem*, QTimer*>> proyectilesActivos;
    QVector<QPair<QGraphicsEllipseItem*, QTimer*>> obtenerProyectilesActivos() const {
        return proyectilesActivos;
    }
    void eliminarProyectiles();

private:
    void cargarAnimaciones();
    void animarMovimiento();
    void iniciarDisparo();      // activa el disparo continuo
    void crearDisparo();        // lanza una bala

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

    bool disparando = false;
    QTimer* animacionTimer;
    QTimer* timerDisparo;

};

#endif // ENEMIGO_H
