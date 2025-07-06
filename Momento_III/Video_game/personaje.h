#ifndef PERSONAJE_H
#define PERSONAJE_H

#include <QGraphicsPixmapItem>
#include <QTimer>

// Clase base abstracta para personajes jugables o enemigos
class Personaje : public QObject, public QGraphicsPixmapItem {
    Q_OBJECT

public:
    // Constructor base
    Personaje(float x, float y, float ancho, float alto);

    // Métodos virtuales puros para comportamiento específico
    virtual void mover() = 0;
    virtual void disparar(QGraphicsScene* escena) = 0;

//NIVEL 2:

    // Nuevos métodos para nivel 2 (cenital)
    virtual void mover2(const QString& direccion);
    virtual void disparar2(QGraphicsScene* escena) = 0;
    bool colisionaConMuro(const QPointF& nuevaPos, QGraphicsScene* escena);

protected:
    // Atributos físicos y visuales del personaje
    float posX, posY;       // Posición
    float velocidadY;       // Velocidad vertical
    float gravedad;         // Intensidad de la caída
    float ancho, alto;      // Dimensiones

    int frameActual = 0;
    QVector<QPixmap> framesArriba;
    QVector<QPixmap> framesAbajo;
    QVector<QPixmap> framesIzquierda;
    QVector<QPixmap> framesDerecha;
};

#endif // PERSONAJE_H
