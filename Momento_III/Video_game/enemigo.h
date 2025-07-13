#ifndef ENEMIGO_H
#define ENEMIGO_H

#include "personaje.h"
#include <QVector>
#include <QPixmap>
#include <QTimer>
#include <QGraphicsScene>

class Goku;

/**
 * @brief La clase Enemigo representa un personaje hostil que puede patrullar y disparar.
 *
 * Hereda de Personaje e implementa comportamiento de patrullaje, detección de Goku y disparo.
 * Incluye lógica para animaciones, pausa/reanudación, y manejo de proyectiles para dos niveles distintos.
 */
class Enemigo : public Personaje {
    Q_OBJECT

public:
    /**
     * @brief Constructor del Enemigo.
     * @param x Posición X inicial.
     * @param y Posición Y inicial.
     * @param ancho Ancho del sprite.
     * @param alto Alto del sprite.
     * @param gokuRef Puntero a Goku (para detección).
     * @param esNivel2 Indica si es el nivel 2 (modo laberinto).
     */
    Enemigo(float x, float y, float ancho, float alto, Goku* gokuRef, bool esNivel2 = false);

    /** @brief Movimiento del enemigo (nivel 1). */
    void mover() override;

    /** @brief Disparo del enemigo si detecta a Goku (nivel 1). */
    void disparar(QGraphicsScene* escena) override;

    /** @brief Verifica si el enemigo está actualmente disparando. */
    bool estaDisparando() const { return estado == Disparando; }

    /** @brief Elimina todos los proyectiles activos del enemigo. */
    void eliminarProyectiles();

    /** @brief Detiene todos los proyectiles activos. */
    void detenerProyectiles();

    /** @brief Reanuda todos los proyectiles activos. */
    void reanudarProyectiles();

    /** @brief Cambia el estado de pausa del enemigo. */
    void setPausado(bool pausa);

    QVector<QPair<QGraphicsEllipseItem*, QTimer*>> proyectilesActivos; ///< Proyectiles activos en escena

    /**
     * @brief Carga las animaciones del enemigo para el nivel 2.
     * @param tileW Ancho de cada frame.
     * @param tileH Alto de cada frame.
     */
    void cargarAnimacionesNivel2(int tileW, int tileH);

    QTimer* timerDisparo2 = nullptr; ///< Temporizador para controlar disparos en nivel 2
    int tiempoEsperaDisparo = 2000; ///< Tiempo de espera entre disparos (ms)

    /** @brief Obtiene el frame actual de la animación. */
    int getFrameActual() const;

    /** @brief Devuelve el puntero a Goku. */
    Personaje* getGoku() const;

    /** @brief Activa o desactiva el disparo en el nivel 2. */
    void setDisparandoNivel2(bool valor);

    /** @brief Indica si está disparando en nivel 2. */
    bool isDisparandoNivel2() const;

    /** @brief Detiene todas las actividades del enemigo. */
    void detener();

    /** @brief Reanuda todas las actividades del enemigo. */
    void reanudar();

private slots:
    /** @brief Patrullaje automático en el nivel 2. */
    void patrullar();

private:
    /** @brief Carga las animaciones del enemigo para el nivel 1. */
    void cargarAnimaciones();

    /** @brief Inicia el proceso de disparo (nivel 1). */
    void iniciarDisparo();

    /** @brief Crea el proyectil real y lo lanza. */
    void crearDisparo();

    /** @brief Estados posibles del enemigo. */
    enum EstadoEnemigo { Patrullando, Disparando, Muerto };
    EstadoEnemigo estado = Patrullando; ///< Estado actual del enemigo

    Goku* gokuDetectado = nullptr; ///< Referencia a Goku detectado (nivel 1)

    int direccion; ///< Dirección de movimiento (-1: izquierda, 1: derecha)
    int frameActual; ///< Frame actual de animación
    int contador; ///< Contador para control de animación
    int velocidadAnimacion; ///< Velocidad de cambio de frame
    float velocidadMovimiento; ///< Velocidad de patrullaje

    bool pausado = false; ///< Indica si está pausado
    bool disparando = false; ///< Indica si está disparando

    QTimer* timerDisparo; ///< Temporizador de disparo (nivel 1)

    // Nivel 2
    QTimer* timerPatrulla; ///< Temporizador de patrullaje (nivel 2)
    QString direccionActual; ///< Dirección actual en nivel 2 ("arriba", "izquierda"...)
    Personaje* goku; ///< Referencia a Goku como Personaje (nivel 2)
    bool disparandoNivel2 = false; ///< Indica si está disparando (nivel 2)
};

#endif // ENEMIGO_H
