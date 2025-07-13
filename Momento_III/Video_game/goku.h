#ifndef GOKU_H
#define GOKU_H

#include "personaje.h"
#include <QVector>
#include <QPixmap>
#include <QGraphicsScene>

class Enemigo;

/**
 * @brief La clase Goku representa al personaje principal controlado por el jugador.
 *
 * Hereda de Personaje. Contiene comportamiento para moverse, saltar, disparar, animaciones,
 * control de energía, detección de suelo y lógica diferenciada para el nivel 2.
 */
class Goku : public Personaje {
    Q_OBJECT

public:
    /**
     * @brief Constructor de Goku.
     * @param x Posición inicial en X.
     * @param y Posición inicial en Y.
     * @param ancho Ancho del sprite.
     * @param alto Alto del sprite.
     */
    Goku(float x, float y, float ancho, float alto);

    /** @brief Movimiento principal (nivel 1). */
    void mover() override;

    /** @brief Disparo principal (nivel 1). */
    void disparar(QGraphicsScene* escena) override;

    /** @brief Ejecuta un salto hacia arriba. */
    void saltar();

    /** @brief Acelera la caída (por tecla S). */
    void acelerarCaida();

    /** @brief Ejecuta la animación de correr. */
    void animarCorrer();

    /** @brief Ejecuta la animación de caída. */
    void animarCaida();

    /** @brief Ejecuta la animación de disparo. */
    void animarDisparo();

    /** @brief Ejecuta la animación cuando está colgado de la cuerda. */
    void animarCuerda();

    /** @brief Controla los frames de animación durante disparo. */
    void animarDisparoFrame();

    /** @brief Verifica si Goku está disparando. */
    bool estaDisparando() const;

    /** @brief Verifica si Goku puede disparar. */
    bool puedeDisparar() const { return energia >= energiaMaxima; }

    /** @brief Reinicia la energía a cero. */
    void reiniciarEnergia() { energia = 0; }

    /**
     * @brief Aumenta la energía en una cantidad determinada.
     * @param cantidad Cantidad a aumentar.
     */
    void aumentarEnergia(int cantidad) {
        energia += cantidad;
        if (energia >= energiaMaxima) energia = energiaMaxima;
    }

    /** @brief Obtiene la cantidad actual de energía. */
    int obtenerEnergia() const { return energia; }

    /** @brief Verifica si está cayendo. */
    bool estaBajando() const;

    /** @brief Verifica si está tocando el suelo. */
    bool estaEnSuelo() const;

    /** @brief Detiene la caída (por contacto con plataforma). */
    void detenerCaida();

    /** @brief Activa la caída por gravedad. */
    void activarCaida();

    /** @brief Fuerza la caída manual (por tecla S). */
    void forzarCaida();

    /** @brief Verifica si está forzando la caída. */
    bool estaForzandoCaida() const;

    /** @brief Cancela la caída forzada. */
    void cancelarCaidaForzada();

    /**
     * @brief Asigna referencia a lista de enemigos cercanos para interacción.
     * @param lista Puntero a QVector de enemigos.
     */
    void setListaEnemigos(QVector<Enemigo*>* lista);

    bool teclaWSostenida = false; ///< Indica si la tecla W está presionada
    bool cayendoLento = false;    ///< Controla si la caída es lenta (gravedad baja)

    // --- NIVEL 2 ---

    /** @brief Carga las animaciones para el modo laberinto (nivel 2). */
    void cargarAnimacionesNivel2();

    QString direccionActual = "derecha"; ///< Dirección actual (nivel 2)

    /** @brief Activa o desactiva al personaje. */
    void setEnabled(bool enabled) { m_enabled = enabled; }

    /** @brief Verifica si el personaje está activo. */
    bool isEnabled() const { return m_enabled; }

protected:
    /** @brief Captura eventos de teclado. */
    void keyPressEvent(QKeyEvent* event) override;

    // void keyReleaseEvent(QKeyEvent* event) override;

private:
    QVector<QPixmap> framesCorrer;     ///< Frames para animación de correr
    QVector<QPixmap> framesCaer;       ///< Frames para animación de caída
    QVector<QPixmap> framesDisparo;    ///< Frames para animación de disparo
    QPixmap spriteCuerda;              ///< Sprite cuando está colgado de la cuerda

    int frameActual;                   ///< Frame actual de la animación
    int contador;                      ///< Contador para controlar velocidad de animación
    int velocidadAnimacion;            ///< Frecuencia de cambio de frame

    bool disparando;                   ///< Indica si está disparando
    QTimer* disparoTimer;              ///< Temporizador para control de disparo
    int frameDisparoActual;            ///< Frame actual del ciclo de disparo

    bool enSuelo;                      ///< Indica si está tocando el suelo

    int energia = 0;                   ///< Energía actual
    const int energiaMaxima = 100;     ///< Energía máxima que puede acumular

    QVector<Enemigo*>* listaEnemigos = nullptr; ///< Lista de enemigos en escena

    bool forzarCaidaManual = false;    ///< Indica si se está forzando la caída

    /** @brief Carga los sprites desde archivo. */
    void cargarAnimaciones();

    /** @brief Actualiza el sprite visible según el estado y frame actual. */
    void actualizarSprite();

    bool m_enabled = true; ///< Indica si Goku está habilitado en el nivel 2
};

#endif // GOKU_H
