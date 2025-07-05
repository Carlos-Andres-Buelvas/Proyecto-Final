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

    void mover() override;                        // Movimiento del enemigo
    void disparar(QGraphicsScene* escena) override; // Disparo si detecta a Goku
    bool estaDisparando() const { return estado == Disparando; }

    void eliminarProyectiles();                  // Eliminar proyectiles activos (al morir)
    void detenerProyectiles();                   // Pausar proyectiles si el juego está pausado
    void reanudarProyectiles();                  // Reanudar proyectiles
    void setPausado(bool pausa);                 // Cambia estado de pausa

    QVector<QPair<QGraphicsEllipseItem*, QTimer*>> proyectilesActivos; // Proyectiles en escena

private:
    void cargarAnimaciones();                    // Carga sprites de movimiento
    void iniciarDisparo();                       // Inicia ciclo de disparo
    void crearDisparo();                         // Crea el proyectil real

    enum EstadoEnemigo { Patrullando, Disparando, Muerto };
    EstadoEnemigo estado = Patrullando;

    Goku* gokuDetectado = nullptr;               // Referencia a Goku para detección

    QVector<QPixmap> framesIzquierda;            // Animaciones hacia la izquierda
    QVector<QPixmap> framesDerecha;              // Animaciones hacia la derecha

    int direccion;                               // -1: izquierda, 1: derecha
    int frameActual;                             // Frame actual de la animación
    int contador;                                // Para controlar ciclo de animación
    int velocidadAnimacion;                      // Velocidad de animación
    float velocidadMovimiento;                   // Velocidad de patrullaje

    bool pausado = false;                        // Indica si el enemigo está pausado
    bool disparando = false;                     // Si actualmente está disparando

    QTimer* timerDisparo;                        // Temporizador para disparar
};

#endif // ENEMIGO_H
