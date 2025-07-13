#ifndef PERSONAJE_H
#define PERSONAJE_H

#include <QGraphicsPixmapItem>
#include <QTimer>

/**
 * @brief Clase base abstracta para todos los personajes del juego.
 *
 * Sirve como clase padre para `Goku` y `Enemigo`, definiendo las
 * funcionalidades comunes como movimiento, disparo y físicas. Incluye
 * soporte para el segundo nivel con vista cenital.
 */
class Personaje : public QObject, public QGraphicsPixmapItem {
    Q_OBJECT

public:
    /**
     * @brief Constructor base del personaje.
     * @param x Posición X inicial
     * @param y Posición Y inicial
     * @param ancho Ancho del sprite
     * @param alto Alto del sprite
     */
    Personaje(float x, float y, float ancho, float alto);

    /**
     * @brief Método virtual puro para movimiento (nivel 1).
     */
    virtual void mover() = 0;

    /**
     * @brief Método virtual puro para disparo (nivel 1).
     * @param escena Escena donde se agregan los proyectiles
     */
    virtual void disparar(QGraphicsScene* escena) = 0;

    // NIVEL 2:

    /**
     * @brief Mueve al personaje en dirección específica (nivel 2).
     * @param direccion Puede ser "arriba", "abajo", "izquierda", "derecha"
     */
    void mover2(const QString& direccion);

    /**
     * @brief Lanza un disparo con hasta 3 proyectiles activos (nivel 2).
     * @param escena Escena donde se agregan los proyectiles
     */
    void disparar2(QGraphicsScene* escena);

    /**
     * @brief Verifica si una posición nueva colisiona con un muro.
     * @param nuevaPos Nueva posición tentativa
     * @param escena Escena donde se verifica la colisión
     * @return true si colisiona con un muro, false si el movimiento es válido
     */
    bool colisionaConMuro(const QPointF& nuevaPos, QGraphicsScene* escena);

signals:
    /**
     * @brief Señal emitida cuando la energía debe actualizarse visualmente.
     */
    void actualizarBarraEnergiaSignal();

    /**
     * @brief Señal emitida cuando Goku es derrotado por un enemigo.
     */
    void gokuDerrotado();

protected:
    // Atributos físicos y visuales del personaje

    float posX, posY;       ///< Posición actual del personaje
    float velocidadY;       ///< Velocidad vertical (solo nivel 1)
    float gravedad;         ///< Gravedad aplicada (solo nivel 1)
    float ancho, alto;      ///< Dimensiones del personaje

    int frameActual = 0;    ///< Índice del frame actual de animación

    // Animaciones para nivel 2
    QVector<QPixmap> framesArriba;      ///< Sprites al moverse hacia arriba
    QVector<QPixmap> framesAbajo;       ///< Sprites al moverse hacia abajo
    QVector<QPixmap> framesIzquierda;   ///< Sprites al moverse a la izquierda
    QVector<QPixmap> framesDerecha;     ///< Sprites al moverse a la derecha

    // Proyectiles activos y sus timers
    QVector<QPair<QGraphicsEllipseItem*, QTimer*>> proyectilesActivos;

    QString tipo; ///< Tipo de personaje ("goku" o "enemigo")
};

#endif // PERSONAJE_H
